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

#include <concepts>
#include <memory_resource>

#include "sxt/base/container/span.h"
#include "sxt/base/device/event.h"
#include "sxt/base/device/event_utility.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/device/pointer_attributes.h"
#include "sxt/base/device/state.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/type/value_type.h"
#include "sxt/execution/async/future.h"
#include "sxt/execution/device/event_future.h"
#include "sxt/execution/device/synchronization.h"
#include "sxt/memory/management/managed_array.h"

namespace sxt::xendv {
//--------------------------------------------------------------------------------------------------
// make_active_device_viewable_impl
//--------------------------------------------------------------------------------------------------
namespace detail {
template <class T, class F, class Cont>
  requires std::convertible_to<Cont, basct::cspan<T>>
event_future<basct::cspan<T>> make_active_device_viewable_impl(F do_allocate,
                                                               const Cont& cont) noexcept {
  basct::cspan<T> data{cont};
  if (data.empty()) {
    return event_future<basct::cspan<T>>{std::move(data)};
  }
  auto active_device = basdv::get_device();
  basdv::pointer_attributes attrs;
  basdv::get_pointer_attributes(attrs, data.data());
  if (attrs.device == active_device || attrs.kind == basdv::pointer_kind_t::managed) {
    return event_future<basct::cspan<T>>{std::move(data)};
  }
  basct::span<T> data_p{do_allocate(data.size()), data.size()};
  basdv::stream stream;
  basdv::async_memcpy_to_device(data_p.data(), data.data(), sizeof(T) * data.size(), attrs, stream);
  basdv::event event;
  basdv::record_event(event, stream);
  computation_handle handle;
  handle.add_stream(std::move(stream));
  return event_future<basct::cspan<T>>{data_p, active_device, std::move(event), std::move(handle)};
}
} // namespace detail

//--------------------------------------------------------------------------------------------------
// make_active_device_viewable
//--------------------------------------------------------------------------------------------------
template <class T, class Cont>
  requires std::convertible_to<Cont, basct::cspan<T>>
event_future<basct::cspan<T>> make_active_device_viewable(memmg::managed_array<T>& data_p,
                                                          const Cont& cont) noexcept {
  auto do_allocate = [&](size_t n) noexcept {
    data_p.resize(n);
    return data_p.data();
  };
  return detail::make_active_device_viewable_impl<T>(do_allocate, cont);
}

/**
 * Use winked-out allocations.
 *
 * Note: Be sure to use this with a compatible allocator.
 * See section 3 of https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0089r1.pdf
 */
template <class Cont, class T = bast::value_type_t<Cont>>
  requires std::convertible_to<Cont, basct::cspan<T>>
event_future<basct::cspan<T>> make_active_device_viewable(std::pmr::polymorphic_allocator<> alloc,
                                                          const Cont& cont) noexcept {
  auto do_allocate = [&](size_t n) noexcept {
    auto ptr = alloc.allocate_bytes(sizeof(T) * n, alignof(T));
    return static_cast<T*>(ptr);
  };
  return detail::make_active_device_viewable_impl<T>(do_allocate, cont);
}
} // namespace sxt::xendv
