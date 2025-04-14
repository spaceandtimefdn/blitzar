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

#include "sxt/base/container/span.h"
#include "sxt/base/device/pinned_buffer.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/functional/function_ref.h"
#include "sxt/execution/async/future.h"

namespace sxt::xendv {
//--------------------------------------------------------------------------------------------------
// to_device_copier
//--------------------------------------------------------------------------------------------------
class to_device_copier {
public:
  to_device_copier(basct::span<std::byte> dst, const basdv::stream& stream) noexcept;

  template <class Cont>
    requires requires(Cont& dst) {
      requires std::is_lvalue_reference_v<decltype(*dst.data())>;
      { dst.size() } -> std::convertible_to<size_t>;
    }
  to_device_copier(Cont& dst, basdv::stream& stream) noexcept
      : to_device_copier{basct::span<std::byte>{reinterpret_cast<std::byte*>(dst.data()),
                                                dst.size() * sizeof(*dst.data())},
                         stream} {}

  xena::future<> copy(basct::cspan<std::byte> src) noexcept;

private:
  basct::span<std::byte> dst_;
  const basdv::stream& stream_;
  basdv::pinned_buffer active_buffer_;
  basdv::pinned_buffer alt_buffer_;
};

//--------------------------------------------------------------------------------------------------
// copy
//--------------------------------------------------------------------------------------------------
template <class Cont>
  requires requires(const Cont& src) {
    requires std::is_pointer_v<decltype(src.data())>;
    { src.size() } -> std::convertible_to<size_t>;
  }
xena::future<> copy(to_device_copier& copier, const Cont& src) noexcept {
  return copier.copy(basct::cspan<std::byte>{reinterpret_cast<const std::byte*>(src.data()),
                                             src.size() * sizeof(*src.data())});
}
} // namespace sxt::xendv
