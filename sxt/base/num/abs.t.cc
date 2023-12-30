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
#include "sxt/base/num/abs.h"

#include "sxt/base/test/unit_test.h"
#include "sxt/base/type/int.h"

using namespace sxt;
using namespace sxt::basn;

TEST_CASE("we can compute the absolute value of numbers") {
  SECTION("we can compute the absolute value of numbers up to 8 bytes") {
    REQUIRE(abs(1) == 1);
    REQUIRE(abs(-1) == 1);
    REQUIRE(abs(-1ll) == 1ll);
  }

  SECTION("we can compute the absolute value of numbers larger than 8 bytes") {
    REQUIRE(abs(int128_t{-1}) == 1);
    REQUIRE(abs(int128_t{1}) == 1);
    REQUIRE(abs(int128_t{-2}) == 2);
    REQUIRE(abs(int128_t{2}) == 2);
  }
}
