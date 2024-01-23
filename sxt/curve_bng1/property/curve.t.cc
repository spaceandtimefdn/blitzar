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
#include "sxt/curve_bng1/property/curve.h"

#include "sxt/base/test/unit_test.h"
#include "sxt/curve_bng1/constant/b.h"
#include "sxt/curve_bng1/constant/generator.h"
#include "sxt/curve_bng1/type/element_affine.h"
#include "sxt/curve_bng1/type/element_p2.h"
#include "sxt/field25/constant/one.h"
#include "sxt/field25/operation/mul.h"
#include "sxt/field25/operation/neg.h"

using namespace sxt;
using namespace sxt::cn1p;

TEST_CASE("an affine element") {
  SECTION("equal to the generator is on the curve") {
    REQUIRE(is_on_curve(cn1cn::generator_affine_v));
  }

  SECTION("equal to the identity is on the curve") {
    REQUIRE(is_on_curve(cn1t::element_affine::identity()));
  }

  SECTION("equal to (1,1) is not on the curve") {
    constexpr cn1t::element_affine one_one{f25cn::one_v, f25cn::one_v, false};

    REQUIRE(!is_on_curve(one_one));
  }
}

TEST_CASE("a projective element") {
  SECTION("equal to the generator is on the curve") { REQUIRE(is_on_curve(cn1cn::generator_p2_v)); }

  SECTION("equal to the generator projected by z is on the curve") {
    // z is arbitrarily chosen to be 3 in Montgomery form for this section of the test.
    constexpr f25t::element z{cn1cn::b_v};
    f25t::element x_projected;
    f25t::element y_projected;
    f25o::mul(x_projected, cn1cn::generator_p2_v.X, z);
    f25o::mul(y_projected, cn1cn::generator_p2_v.Y, z);
    cn1t::element_p2 generator_projected{x_projected, y_projected, z};

    REQUIRE(is_on_curve(generator_projected));

    f25t::element neg_y;
    f25o::neg(neg_y, generator_projected.Y);
    generator_projected.Y = neg_y;

    REQUIRE(is_on_curve(generator_projected));

    f25t::element neg_x;
    f25o::neg(neg_x, generator_projected.X);
    generator_projected.X = neg_x;

    REQUIRE(!is_on_curve(generator_projected));
  }

  SECTION("equal to the identity is on the curve") {
    REQUIRE(is_on_curve(cn1t::element_p2::identity()));
  }

  SECTION("equal to (1,1,1) is not on the curve") {
    constexpr cn1t::element_p2 one_one{f25cn::one_v, f25cn::one_v, f25cn::one_v};

    REQUIRE(!is_on_curve(one_one));
  }
}
