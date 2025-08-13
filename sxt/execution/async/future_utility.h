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

#include <vector>

#include "sxt/execution/async/future.h"

namespace sxt::xena {
//--------------------------------------------------------------------------------------------------
// await_all
//--------------------------------------------------------------------------------------------------
// await multiple futures
//
// Note: This is to work around https://github.com/spaceandtimefdn/blitzar/issues/200
future<> await_all(std::vector<future<>> futs) noexcept;

template <class... Tx>
  requires(std::is_same_v<Tx, future<>> && ...)
future<> await_all(Tx&&... futs) noexcept {
  std::vector<future<>> fut_vec;
  fut_vec.reserve(sizeof...(futs));
  (fut_vec.emplace_back(std::move(futs)), ...);
  return await_all(std::move(fut_vec));
}
} // namespace sxt::xena
