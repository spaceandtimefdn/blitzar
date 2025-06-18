/** Proofs GPU - Space and Time's cryptographic proof algorithms on the CPU and GPU.
 *
 * Copyright 2023-present Space and Time Labs, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <algorithm>
#include <cstddef>

#include "sxt/base/container/span.h"
#include "sxt/base/curve/element.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/device/property.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/error/assert.h"
#include "sxt/base/iterator/split.h"
#include "sxt/base/num/divide_up.h"
#include "sxt/execution/async/coroutine.h"
#include "sxt/execution/async/future.h"
#include "sxt/execution/device/device_viewable.h"
#include "sxt/execution/device/for_each.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/async_device_resource.h"
#include "sxt/memory/resource/pinned_resource.h"
#include "sxt/multiexp/bucket_method/accumulation_kernel.h"
#include "sxt/multiexp/bucket_method/combination_kernel.h"
#include "sxt/multiexp/bucket_method/fold_kernel.h"

namespace sxt::mtxbk {
//--------------------------------------------------------------------------------------------------
// make_exponents_viewable
//--------------------------------------------------------------------------------------------------
basct::cspan<const uint8_t>
make_exponents_viewable(memmg::managed_array<uint8_t>& exponents_viewable_data,
                        basct::cspan<const uint8_t*> exponents, const basit::index_range& rng,
                        const basdv::stream& stream) noexcept;

//--------------------------------------------------------------------------------------------------
// accumulate_buckets_impl
//--------------------------------------------------------------------------------------------------
template <bascrv::element T>
xena::future<> accumulate_buckets_impl(basct::span<T> bucket_sums, basct::cspan<T> generators,
                                       basct::cspan<const uint8_t*> exponents,
                                       basit::index_range rng) noexcept {
  unsigned n = rng.size();
  auto num_outputs = exponents.size();
  auto num_blocks = std::min(192u, n);

  // hard code parameters for now
  static constexpr unsigned num_bucket_groups = 32;
  static constexpr unsigned bucket_group_size = 255;

  basdv::stream stream;
  memr::async_device_resource resource{stream};

  // make generators accessible to the active device
  memmg::managed_array<T> generators_viewable_data{&resource};
  auto generators_viewable = xendv::make_active_device_viewable(
      generators_viewable_data, generators.subspan(rng.a(), rng.size()));

  // make exponents accessible to the active device
  memmg::managed_array<uint8_t> exponents_viewable_data{&resource};
  auto exponents_viewable =
      make_exponents_viewable(exponents_viewable_data, exponents, rng, stream);

  // accumulate generators into buckets of partial sums
  xendv::synchronize_event(stream, generators_viewable);
  memmg::managed_array<T> partial_bucket_sums{bucket_sums.size() * num_blocks, &resource};
  bucket_accumulate<<<dim3(num_blocks, num_outputs, 1), num_bucket_groups, 0, stream>>>(
      partial_bucket_sums.data(), generators_viewable.value().data(), exponents_viewable.data(), n);
  generators_viewable_data.reset();
  exponents_viewable_data.reset();

  // combine partial sums
  memmg::managed_array<T> bucket_sums_dev{bucket_sums.size(), &resource};
  combine_partial_bucket_sums<<<dim3(bucket_group_size, num_outputs, 1), num_bucket_groups, 0,
                                stream>>>(bucket_sums_dev.data(), partial_bucket_sums.data(),
                                          num_blocks);
  partial_bucket_sums.reset();
  basdv::async_copy_device_to_host(bucket_sums, bucket_sums_dev, stream);
  co_await xendv::await_stream(stream);
}

template <bascrv::element T>
xena::future<> accumulate_buckets_impl(basct::span<T> bucket_sums, basct::cspan<T> generators,
                                       basct::cspan<const uint8_t*> exponents,
                                       size_t split_factor) noexcept {
  // hard code parameters for now
  static constexpr unsigned bucket_group_size = 255;
  static constexpr unsigned num_bucket_groups = 32;

  auto num_outputs = exponents.size();
  SXT_DEBUG_ASSERT(
      // clang-format off
      bucket_sums.size() == bucket_group_size * num_bucket_groups * num_outputs &&
      (bucket_sums.empty() || basdv::is_active_device_pointer(bucket_sums.data()))
      // clang-format on
  );

  // Pick some reasonable values for min and max chunk size so that
  // we don't run out of GPU memory or split computations that are
  // too small.
  //
  // Note: These haven't been informed by much benchmarking. I'm
  // sure there are better values. This is just putting in some
  // ballpark estimates to get started.
  size_t min_chunk_size = 1ull << 10u;
  size_t max_chunk_size = 1ull << 20u;
  if (num_outputs > 0) {
    max_chunk_size = basn::divide_up(max_chunk_size, num_outputs);
    min_chunk_size *= num_outputs;
    min_chunk_size = std::min(max_chunk_size, min_chunk_size);
  }
  basit::split_options split_options{
      .min_chunk_size = min_chunk_size,
      .max_chunk_size = max_chunk_size,
      .split_factor = split_factor,
  };
  auto [first, last] = basit::split(basit::index_range{0, generators.size()}, split_options);
  auto num_chunks = std::distance(first, last);

  memmg::managed_array<T> partial_bucket_sums_data{memr::get_pinned_resource()};
  basct::span<T> partial_bucket_sums;
  if (num_chunks > 1) {
    partial_bucket_sums_data.resize(num_bucket_groups * bucket_group_size * num_outputs *
                                    num_chunks);
    partial_bucket_sums = partial_bucket_sums_data;
  } else {
    partial_bucket_sums = bucket_sums;
  }

  size_t step = num_bucket_groups * bucket_group_size * num_outputs;
  size_t i = 0;
  co_await xendv::concurrent_for_each(first, last, [&](const basit::index_range& rng) noexcept {
    return accumulate_buckets_impl(partial_bucket_sums.subspan(step * i++, step), generators,
                                   exponents, rng);
  });
  if (num_chunks <= 1) {
    co_return;
  }
  basdv::stream stream;
  memr::async_device_resource resource{stream};
  memmg::managed_array<T> partial_bucket_sums_dev{partial_bucket_sums.size(), &resource};
  basdv::async_copy_host_to_device(partial_bucket_sums_dev, partial_bucket_sums, stream);
  segmented_left_fold_partial_bucket_sums<<<dim3(bucket_group_size, num_outputs, 1),
                                            num_bucket_groups, 0, stream>>>(
      bucket_sums.data(), partial_bucket_sums_dev.data(), bucket_sums.size(),
      partial_bucket_sums_dev.size());
  co_await xendv::await_stream(stream);
}

//--------------------------------------------------------------------------------------------------
// accumulate_buckets
//--------------------------------------------------------------------------------------------------
/**
 * Accumulate generators into buckets, splitting the work across available devices.
 *
 * This function corresponds roughly to the 1st loop of Algorithm 1 described in
 *
 *    PipeMSM: Hardware Acceleration for Multi-Scalar Multiplication
 *    https://eprint.iacr.org/2022/999.pdf
 */
template <bascrv::element T>
xena::future<> accumulate_buckets(basct::span<T> bucket_sums, basct::cspan<T> generators,
                                  basct::cspan<const uint8_t*> exponents) noexcept {
  return accumulate_buckets_impl(bucket_sums, generators, exponents, basdv::get_num_devices());
}
} // namespace sxt::mtxbk
