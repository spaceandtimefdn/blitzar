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
#include "sxt/base/device/pinned_buffer_pool.h"

#include <cuda_runtime.h>

#include <cassert>

#include "sxt/base/device/pinned_buffer_handle.h"
#include "sxt/base/error/panic.h"

namespace sxt::basdv {
//--------------------------------------------------------------------------------------------------
// new_handle
//--------------------------------------------------------------------------------------------------
static pinned_buffer_handle* new_handle() noexcept {
  auto res = new pinned_buffer_handle{};
  auto rcode = cudaMallocHost(&res->ptr, pinned_buffer_size);
  if (rcode != cudaSuccess) {
    baser::panic("cudaMallocHost failed: {}", cudaGetErrorString(rcode));
  }
  return res;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
pinned_buffer_pool::pinned_buffer_pool(size_t initial_size) noexcept {
  for (size_t i = 0; i < initial_size; ++i) {
    auto h = new_handle();
    this->release_handle(h);
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
pinned_buffer_pool::~pinned_buffer_pool() noexcept {
  while (head_ != nullptr) {
    auto rcode = cudaFreeHost(head_->ptr);
    if (rcode != cudaSuccess) {
      baser::panic("cudaFreeHost failed: {}", cudaGetErrorString(rcode));
    }
    auto next = head_->next;
    delete head_;
    head_ = next;
  }
}

//--------------------------------------------------------------------------------------------------
// acquire_handle
//--------------------------------------------------------------------------------------------------
pinned_buffer_handle* pinned_buffer_pool::acquire_handle() noexcept {
  if (head_ == nullptr) {
    head_ = new_handle();
  }
  auto res = head_;
  head_ = res->next;
  res->next = nullptr;
  return res;
}

//--------------------------------------------------------------------------------------------------
// release_handle
//--------------------------------------------------------------------------------------------------
void pinned_buffer_pool::release_handle(pinned_buffer_handle* handle) noexcept {
  assert(handle->next == nullptr);
  handle->next = head_;
  head_ = handle;
}

//--------------------------------------------------------------------------------------------------
// size
//--------------------------------------------------------------------------------------------------
size_t pinned_buffer_pool::size() const noexcept {
  size_t res = 0;
  auto h = head_;
  while (h != nullptr) {
    ++res;
    h = h->next;
  }
  return res;
}

//--------------------------------------------------------------------------------------------------
// get_pinned_buffer_pool
//--------------------------------------------------------------------------------------------------
/**
 * Access the thread_local pinned pool.
 */
pinned_buffer_pool* get_pinned_buffer_pool(size_t initial_size) noexcept {
  // Allocate a thread local pool that's available for the duration of the process.
  static thread_local auto pool = new pinned_buffer_pool{initial_size};
  return pool;
}
} // namespace sxt::basdv
