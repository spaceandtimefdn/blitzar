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
#include "sxt/field25/property/lexicographically_largest.h"

#include "sxt/base/test/unit_test.h"
#include "sxt/field25/base/montgomery.h"
#include "sxt/field25/constant/one.h"
#include "sxt/field25/constant/zero.h"
#include "sxt/field25/type/element.h"

using namespace sxt;
using namespace sxt::f25p;

TEST_CASE("lexicographically largest correctly identifies") {
  SECTION("zero is not largest") { REQUIRE(!lexicographically_largest(f25cn::zero_v)); }

  SECTION("one is not largest") { REQUIRE(!lexicographically_largest(f25cn::one_v)); }

  SECTION("(p_v-1)/2 in Montgomery form is not largest") {
    constexpr f25t::element e{0x9e10460b6c3e7ea3, 0xcbc0b548b438e546, 0xdc2822db40c0ac2e,
                              0x183227397098d014};
    f25t::element e_montgomery;
    f25b::to_montgomery_form(e_montgomery.data(), e.data());

    REQUIRE(!lexicographically_largest(e_montgomery));
  }

  SECTION("((p_v-1)/2)+1 in Montgomery form is largest") {
    constexpr f25t::element e{0x9e10460b6c3e7ea4, 0xcbc0b548b438e546, 0xdc2822db40c0ac2e,
                              0x183227397098d014};
    f25t::element e_montgomery;
    f25b::to_montgomery_form(e_montgomery.data(), e.data());

    REQUIRE(lexicographically_largest(e_montgomery));
  }
}
