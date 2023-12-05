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
#include "sxt/algorithm/iteration/transform.h"

#include <vector>

#include "sxt/base/test/unit_test.h"
#include "sxt/execution/device/device_viewable.h"
#include "sxt/execution/schedule/scheduler.h"

using namespace sxt;
using namespace sxt::algi;

TEST_CASE("we can transform contigous regions of memory") {
  std::vector<double> res;
  basit::chunk_options chunk_options;

  SECTION("we handle the empty case") {
    auto f = [] __device__ __host__(double& x) noexcept { x *= 2; };
    auto fut = transform(res, chunk_options, f, res);
    REQUIRE(fut.ready());
  }

  SECTION("we can transform a vector with a single element") {
    res = {123};
    auto f = [] __device__ __host__(double& x) noexcept { x *= 2; };
    auto fut = transform(res, chunk_options, f, res);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 246);
  }

  SECTION("we can transform two vectors") {
    res = {2};
    std::vector<double> y = {4};
    auto f = [] __device__ __host__(double& x, double& y) noexcept { x = x + y; };
    auto fut = transform(res, chunk_options, f, res, y);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 6);
  }

  SECTION("we can split transform across multiple chunks") {
    res = {3, 5};
    auto f = [] __device__ __host__(double& x) noexcept { x *= 2; };
    chunk_options.max_size = 1;
    auto fut = transform(res, chunk_options, f, res);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 6);
    REQUIRE(res[1] == 10);
  }

  SECTION("we can allocate memory for each device chunk") {
    std::vector<int> xs = {3, 2, 1};
    struct functor {
      const int* xs;

      __device__ __host__ void operator()(double& x) const noexcept { x *= xs[0] * xs[1] * xs[2]; };
    };

    auto make_f = [&](std::pmr::polymorphic_allocator<> alloc,
                      basdv::stream& stream) noexcept -> xena::future<functor> {
      auto xs_dev = co_await xendv::make_active_device_viewable(alloc, xs);
      co_return functor{
          .xs = xs_dev.data(),
      };
    };

    res = {3, 4};
    chunk_options.max_size = 1;
    auto fut = transform(res, chunk_options, make_f, res);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 18);
    REQUIRE(res[1] == 24);
  }
}
