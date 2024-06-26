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
#include "sxt/curve_gk/operation/mul_by_3b.h"

#include "sxt/base/test/unit_test.h"
#include "sxt/fieldgk/constant/one.h"
#include "sxt/fieldgk/type/element.h"
#include "sxt/fieldgk/type/literal.h"

using namespace sxt;
using namespace sxt::cgko;
using fgkt::operator""_fgk;

TEST_CASE("multiply by 3b") {
  SECTION("returns nine if one in Montgomery form is the input") {
    fgkt::element ret;

    mul_by_3b(ret, fgkcn::one_v);

    REQUIRE(0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593efffffce_fgk == ret);
  }
}
