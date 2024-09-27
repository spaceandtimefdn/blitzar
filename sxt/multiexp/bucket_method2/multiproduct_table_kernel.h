/** Proofs GPU - Space and Time's cryptographic proof algorithms on the CPU and GPU.
 *
 * Copyright 2024-present Space and Time Labs, Inc.
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

#include <utility>

#include "cub/cub.cuh"
#include "sxt/algorithm/block/runlength_count.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/num/constexpr_switch.h"
#include "sxt/base/num/divide_up.h"
#include "sxt/multiexp/bucket_method2/constants.h"

namespace sxt::mtxbk2 {
//--------------------------------------------------------------------------------------------------
// multiproduct_table_kernel
//--------------------------------------------------------------------------------------------------
template <uint16_t NumThreads, uint16_t ItemsPerThread, unsigned BitWidth>
__global__ void multiproduct_table_kernel(uint16_t* __restrict__ bucket_counts,
                                          uint16_t* __restrict__ indexes,
                                          const uint8_t* __restrict__ bytes, unsigned n) {
  uint16_t thread_index = threadIdx.x;
  auto digit_index = blockIdx.x;
  auto output_index = blockIdx.y;
  auto num_digits = gridDim.x;
  auto num_buckets_per_digit = (1u << BitWidth) - 1u;

  // algorithms and shared memory
  using RadixSort = cub::BlockRadixSort<uint8_t, NumThreads, ItemsPerThread, uint16_t>;
  using RunlengthCount = algbk::runlength_count<uint8_t, uint16_t, NumThreads, (1u << BitWidth)>;
  __shared__ union {
    RadixSort::TempStorage sort;
    RunlengthCount::temp_storage count;
  } temp_storage;

  // adjust pointers
  bucket_counts += digit_index * num_buckets_per_digit;
  bucket_counts += output_index * num_digits * num_buckets_per_digit;
  indexes += digit_index * n;
  indexes += output_index * num_digits * n;
  bytes += digit_index * n;
  bytes += output_index * num_digits * n;

  // load bytes
  uint8_t keys[ItemsPerThread];
  uint16_t values[ItemsPerThread];
  for (uint16_t i = 0; i < ItemsPerThread; ++i) {
    auto index = thread_index + i * NumThreads;
    if (index < n) {
      keys[i] = bytes[index];
      values[i] = index;
    } else {
      keys[i] = 0;
      values[i] = 0;
    }
  }

  // sort
  RadixSort(temp_storage.sort).Sort(keys, values);
  __syncthreads();

  // count
  auto counts = RunlengthCount(temp_storage.count).count(keys);
  __syncthreads();

  // write counts
  for (unsigned i = thread_index; i < num_buckets_per_digit; i += NumThreads) {
    bucket_counts[i] = counts[i + 1];
  }

  // write indexes
  auto zero_count = counts[0];
  for (unsigned i = 0; i < ItemsPerThread; ++i) {
    auto index = i + thread_index * ItemsPerThread;
    if (index >= zero_count) {
      indexes[index - zero_count] = values[i];
    }
  }
}

//--------------------------------------------------------------------------------------------------
// fit_multiproduct_table_kernel
//--------------------------------------------------------------------------------------------------
template <class F> void fit_multiproduct_table_kernel(F f, unsigned n) noexcept {
  SXT_RELEASE_ASSERT(n <= max_multiexponentiation_length_v);
  basn::constexpr_switch<1, max_multiexponentiation_length_v / 128u + 1u>(
      basn::divide_up(n, 128u), [&]<unsigned K>(std::integral_constant<unsigned, K>) noexcept {
        return f(std::integral_constant<unsigned, 128>{}, std::integral_constant<unsigned, K>{});
      });
}
} // namespace sxt::mtxbk2
