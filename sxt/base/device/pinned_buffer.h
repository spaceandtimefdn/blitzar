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

#include <cstddef>

#include "sxt/base/container/span.h"
#include "sxt/base/device/pinned_buffer_handle.h"

namespace sxt::basdv {
//--------------------------------------------------------------------------------------------------
// pinned_buffer
//--------------------------------------------------------------------------------------------------
class pinned_buffer {
public:
  pinned_buffer() noexcept = default;

  explicit pinned_buffer(size_t size) noexcept;

  pinned_buffer(const pinned_buffer&) noexcept = delete;
  pinned_buffer(pinned_buffer&& other) noexcept;

  ~pinned_buffer() noexcept;

  pinned_buffer& operator=(const pinned_buffer&) noexcept = delete;
  pinned_buffer& operator=(pinned_buffer&& other) noexcept;

  bool empty() const noexcept { return size_ == 0; }

  bool full() const noexcept { return size_ == this->capacity(); }

  size_t size() const noexcept { return size_; }

  static size_t capacity() noexcept;

  void* data() noexcept {
    if (handle_ == nullptr) {
      return nullptr;
    }
    return handle_->ptr;
  }

  const void* data() const noexcept {
    if (handle_ == nullptr) {
      return nullptr;
    }
    return handle_->ptr;
  }

  void resize(size_t size) noexcept;

  basct::cspan<std::byte> fill_from_host(basct::cspan<std::byte> src) noexcept;

  void reset() noexcept;

private:
  pinned_buffer_handle* handle_ = nullptr;
  size_t size_ = 0;
};
} // namespace sxt::basdv
