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

#include <cerrno>
#include <cstring>
#include <fstream>
#include <string_view>

#include "sxt/base/container/span_utility.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/error/assert.h"
#include "sxt/base/error/panic.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/pinned_resource.h"
#include "sxt/multiexp/pippenger2/partition_table_accessor.h"

namespace sxt::mtxpp2 {
//--------------------------------------------------------------------------------------------------
// in_memory_partition_table_accessor
//--------------------------------------------------------------------------------------------------
template <bascrv::element T>
class in_memory_partition_table_accessor final : public partition_table_accessor<T> {
public:
  explicit in_memory_partition_table_accessor(std::string_view filename) noexcept
      : table_{memr::get_pinned_resource()} {
    std::ifstream in{filename, std::ios::binary};
    if (!in.good()) {
      baser::panic("failed to open {}: {}", filename, std::strerror(errno));
    }
    auto pos = in.tellg();
    in.seekg(0, std::ios::end);
    auto size = in.tellg() - pos;
    in.seekg(pos);
    SXT_RELEASE_ASSERT(size % sizeof(T) == 0);
    table_.resize(size / sizeof(T));
    in.read(reinterpret_cast<char*>(table_.data()), size);
  }

  void async_copy_precomputed_sums_to_device(basct::span<T> dest, bast::raw_stream_t stream,
                                             unsigned first) const noexcept override {
    SXT_DEBUG_ASSERT(
        // clang-format off
         table_.size() >= dest.size() + first &&
         basdv::is_active_device_pointer(dest.data())
        // clang-format on
    );
    basdv::async_copy_host_to_device(dest, basct::subspan(table_, first, dest.size()), stream);
  }

private:
  memmg::managed_array<T> table_;
};
} // namespace sxt::mtxpp2
