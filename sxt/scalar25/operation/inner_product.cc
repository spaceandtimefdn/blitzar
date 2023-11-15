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
#include "sxt/scalar25/operation/inner_product.h"

#include <algorithm>

#include "sxt/algorithm/reduction/reduction.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/device/property.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/error/assert.h"
#include "sxt/base/iterator/index_range.h"
#include "sxt/base/iterator/index_range_iterator.h"
#include "sxt/base/iterator/index_range_utility.h"
#include "sxt/execution/async/coroutine.h"
#include "sxt/execution/device/device_viewable.h"
#include "sxt/execution/device/for_each.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/async_device_resource.h"
#include "sxt/scalar25/operation/accumulator.h"
#include "sxt/scalar25/operation/mul.h"
#include "sxt/scalar25/operation/muladd.h"
#include "sxt/scalar25/operation/product_mapper.h"

namespace sxt::s25o {
//--------------------------------------------------------------------------------------------------
// async_inner_product_partial
//--------------------------------------------------------------------------------------------------
static xena::future<s25t::element>
async_inner_product_partial(basct::cspan<s25t::element> lhs,
                            basct::cspan<s25t::element> rhs) noexcept {
  auto n = lhs.size();
  basdv::stream stream;
  memr::async_device_resource resource{stream};
  memmg::managed_array<s25t::element> lhs_device_data{&resource};
  auto lhs_fut = xendv::make_active_device_viewable(lhs_device_data, lhs);
  memmg::managed_array<s25t::element> rhs_device_data{&resource};
  auto rhs_dev = co_await xendv::make_active_device_viewable(rhs_device_data, rhs);
  auto lhs_dev = co_await std::move(lhs_fut);
  co_return co_await algr::reduce<accumulator>(std::move(stream),
                                               product_mapper{lhs_dev.data(), rhs_dev.data()},
                                               static_cast<unsigned int>(n));
}

//--------------------------------------------------------------------------------------------------
// inner_product
//--------------------------------------------------------------------------------------------------
void inner_product(s25t::element& res, basct::cspan<s25t::element> lhs,
                   basct::cspan<s25t::element> rhs) noexcept {
  auto n = std::min(lhs.size(), rhs.size());
  SXT_DEBUG_ASSERT(n > 0);
  s25o::mul(res, lhs[0], rhs[0]);
  for (size_t i = 1; i < n; ++i) {
    s25o::muladd(res, lhs[i], rhs[i], res);
  }
}

//--------------------------------------------------------------------------------------------------
// async_inner_product_impl
//--------------------------------------------------------------------------------------------------
xena::future<s25t::element> async_inner_product_impl(basct::cspan<s25t::element> lhs,
                                                     basct::cspan<s25t::element> rhs,
                                                     size_t split_factor, size_t min_chunk_size,
                                                     size_t max_chunk_size) noexcept {
  auto n = std::min(lhs.size(), rhs.size());
  SXT_DEBUG_ASSERT(n > 0);
  s25t::element res = s25t::element::identity();

  auto [chunk_first, chunk_last] = basit::split(
      basit::index_range{0, n}.min_chunk_size(min_chunk_size).max_chunk_size(max_chunk_size),
      split_factor);

  co_await xendv::concurrent_for_each(
      chunk_first, chunk_last, [&](const basit::index_range& rng) noexcept -> xena::future<> {
        auto partial_res = co_await async_inner_product_partial(lhs.subspan(rng.a(), rng.size()),
                                                                rhs.subspan(rng.a(), rng.size()));
        s25o::add(res, res, partial_res);
      });
  co_return res;
}

//--------------------------------------------------------------------------------------------------
// async_inner_product
//--------------------------------------------------------------------------------------------------
xena::future<s25t::element> async_inner_product(basct::cspan<s25t::element> lhs,
                                                basct::cspan<s25t::element> rhs) noexcept {

  // Pick some reasonable values for min and max chunk size so that
  // we don't run out of GPU memory or split computations that are
  // too small.
  //
  // Note: These haven't been informed by much benchmarking. I'm
  // sure there are better values. This is just putting in some
  // ballpark estimates to get started.
  size_t min_chunk_size = 4ull << 10u;
  size_t max_chunk_size = 4ull << 20u;

  return async_inner_product_impl(lhs, rhs, basdv::get_num_devices(), min_chunk_size,
                                  max_chunk_size);
}
} // namespace sxt::s25o
