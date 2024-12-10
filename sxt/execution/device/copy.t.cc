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
#include "sxt/execution/device/copy.h"

#include <cstddef>
#include <numeric>
#include <vector>

#include "sxt/base/device/pinned_buffer.h"
#include "sxt/base/device/stream.h"
#include "sxt/base/device/synchronization.h"
#include "sxt/base/test/unit_test.h"
#include "sxt/execution/schedule/scheduler.h"
#include "sxt/memory/resource/managed_device_resource.h"

using namespace sxt;
using namespace sxt::xendv;

TEST_CASE("we can copy strided memory from host to device") {
  const auto bufsize = basdv::pinned_buffer::size();
  std::vector<uint8_t> src;
  std::pmr::vector<uint8_t> dst{memr::get_managed_device_resource()};

  basdv::stream stream;

  SECTION("we can copy empty data") {
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 0, 0);
    REQUIRE(fut.ready());
  }

  SECTION("we can copy a single byte") {
    src = {123};
    dst.resize(1);
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 1, 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(dst[0] == 123);
  }

  SECTION("we can copy with an offset") {
    src = {1, 2};
    dst.resize(1);
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 1, 1);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(dst[0] == 2);
  }

  SECTION("we can copy every other element") {
    src = {1, 2, 3, 4};
    dst.resize(2);
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 2, 1, 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(dst[0] == 1);
    REQUIRE(dst[1] == 3);
  }

  SECTION("we can copy data as large as a single buffer") {
    src.resize(bufsize);
    std::iota(src.begin(), src.end(), 0u);
    dst.resize(bufsize);
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 1, 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(std::vector<uint8_t>(dst.begin(), dst.end()) == src);
  }

  SECTION("we can copy data larger than a single buffer") {
    src.resize(bufsize + 1u);
    std::iota(src.begin(), src.end(), 0u);
    dst.resize(src.size());
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 1, 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(std::vector<uint8_t>(dst.begin(), dst.end()) == src);
  }

  SECTION("we can copy data where a single chunk is larger than a single buffer") {
    src.resize(bufsize + 1u);
    std::iota(src.begin(), src.end(), 0u);
    dst.resize(src.size());
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, src.size(), src.size(), 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(std::vector<uint8_t>(dst.begin(), dst.end()) == src);
  }

  SECTION("we can copy data larger than two buffers") {
    src.resize(2u * bufsize + 1u);
    std::iota(src.begin(), src.end(), 0u);
    dst.resize(src.size());
    auto fut = strided_copy_host_to_device<uint8_t>(dst, stream, src, 1, 1, 0);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    basdv::synchronize_device();
    REQUIRE(std::vector<uint8_t>(dst.begin(), dst.end()) == src);
  }
}
