/** Proofs GPU - Space and Time's cryptographic proof algorithms on the CPU and GPU.
 *
 * Copyright 2025-present Space and Time Labs, Inc.
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

#include <cassert>

#include "sxt/base/curve/element.h"

namespace sxt::mtxbk {
//--------------------------------------------------------------------------------------------------
// segmented_left_fold_partial_bucket_sums
//--------------------------------------------------------------------------------------------------
/**
 * This kernel computes the left fold of partial bucket sums to produce the final bucket sums.
 *
 * The kernel processes a multi-dimensional array of partial bucket sums, where each thread
 * performs a left fold operation on a specific segment of data identified by its (bucket_index,
 * bucket_group_index, output_index) coordinates. The fold operation combines multiple partial
 * results using element-wise addition.
 *
 * Memory layout:
 * - partial_bucket_sums: Array containing all partial sums, organized as multiple "folds" of data
 *   where each fold has size out_size
 * - out: Output array of size out_size that will contain the folded results
 *
 * Parameters:
 * @param[out] out                     Destination array for the folded results
 * @param[in]  partial_bucket_sums     Source array containing partial bucket sums to be folded
 * @param[in]  out_size                Size of the output array (elements)
 * @param[in]  partial_bucket_sum_size Total size of the partial bucket sums array (elements)
 *
 * Thread organization:
 * - Each thread is responsible for folding one specific element
 * - Thread coordinates: (blockIdx.x, threadIdx.x, blockIdx.y) map to (bucket_index,
 * bucket_group_index, output_index)
 * - The kernel should be launched with a grid of dim3(bucket_group_size, num_outputs, 1) and
 *   blockDim of num_bucket_groups
 *
 * Preconditions:
 * - partial_bucket_sum_size must be greater than 0
 * - out_size must be greater than 0
 * - partial_bucket_sum_size must be greater than or equal to out_size
 * - partial_bucket_sum_size must be evenly divisible by out_size
 */
template <bascrv::element T>
__global__ void segmented_left_fold_partial_bucket_sums(T* out, T* partial_bucket_sums,
                                                        unsigned out_size,
                                                        unsigned partial_bucket_sum_size) {
  assert(partial_bucket_sum_size > 0 && out_size > 0);
  assert(partial_bucket_sum_size >= out_size);
  assert(partial_bucket_sum_size % out_size == 0);

  auto bucket_group_size = gridDim.x;
  auto bucket_group_index = threadIdx.x;
  auto num_bucket_groups = blockDim.x;
  auto bucket_index = blockIdx.x;
  auto output_index = blockIdx.y;

  auto data_offset = bucket_index + bucket_group_size * bucket_group_index +
                     bucket_group_size * num_bucket_groups * output_index;

  partial_bucket_sums += data_offset;
  out += data_offset;

  auto num_of_folds = partial_bucket_sum_size / out_size;

  T sum = *partial_bucket_sums;
  partial_bucket_sums += out_size;
  for (unsigned i = 1; i < num_of_folds; ++i) {
    add_inplace(sum, *partial_bucket_sums);
    partial_bucket_sums += out_size;
  }

  *out = sum;
}
}; // namespace sxt::mtxbk
