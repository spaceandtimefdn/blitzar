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
#include "sxt/base/iterator/index_range.h"

#include "sxt/base/error/assert.h"

namespace sxt::basit {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
index_range::index_range(size_t a, size_t b) noexcept
    : index_range{a, b, 1, std::numeric_limits<size_t>::max()} {}

index_range::index_range(size_t a, size_t b, size_t min_chunk_size, size_t max_chunk_size) noexcept
    : a_{a}, b_{b}, min_chunk_size_{min_chunk_size}, max_chunk_size_{max_chunk_size} {
  SXT_DEBUG_ASSERT(
      // clang-format off
      0 <= a && a <= b &&
      0 < min_chunk_size_ && min_chunk_size_ <= max_chunk_size_
      // clang-format on
  );
}

//--------------------------------------------------------------------------------------------------
// min_chunk_size
//--------------------------------------------------------------------------------------------------
index_range index_range::min_chunk_size(size_t val) const noexcept {
  return {
      a_,
      b_,
      val,
      max_chunk_size_,
  };
}

//--------------------------------------------------------------------------------------------------
// max_chunk_size
//--------------------------------------------------------------------------------------------------
index_range index_range::max_chunk_size(size_t val) const noexcept {
  return {
      a_,
      b_,
      min_chunk_size_,
      val,
  };
}
} // namespace sxt::basit
