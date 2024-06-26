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
#include "sxt/curve_gk/operation/add.h"

#include "sxt/base/num/fast_random_number_generator.h"
#include "sxt/base/test/unit_test.h"
#include "sxt/curve_gk/constant/generator.h"
#include "sxt/curve_gk/operation/double.h"
#include "sxt/curve_gk/property/curve.h"
#include "sxt/curve_gk/property/identity.h"
#include "sxt/curve_gk/type/element_affine.h"
#include "sxt/curve_gk/type/element_p2.h"
#include "sxt/fieldgk/operation/mul.h"
#include "sxt/fieldgk/random/element.h"
#include "sxt/fieldgk/type/element.h"

using namespace sxt;
using namespace sxt::cgko;

TEST_CASE("addition with projective elements") {
  SECTION("keeps the identity on the curve") {
    cgkt::element_p2 ret;
    add(ret, cgkt::element_p2::identity(), cgkt::element_p2::identity());

    REQUIRE(cgkp::is_identity(ret));
    REQUIRE(cgkp::is_on_curve(ret));
  }

  SECTION("is commutative") {
    fgkt::element z;
    basn::fast_random_number_generator rng{1, 2};
    fgkrn::generate_random_element(z, rng);

    fgkt::element x;
    fgkt::element y;
    fgko::mul(x, cgkcn::generator_p2_v.X, z);
    fgko::mul(y, cgkcn::generator_p2_v.Y, z);
    const cgkt::element_p2 projected_generator{x, y, z};
    cgkt::element_p2 ret;

    add(ret, cgkt::element_p2::identity(), projected_generator);

    REQUIRE(!cgkp::is_identity(ret));
    REQUIRE(cgkp::is_on_curve(ret));
    REQUIRE(cgkcn::generator_p2_v == ret);

    // Switch summands
    add(ret, projected_generator, cgkt::element_p2::identity());

    REQUIRE(!cgkp::is_identity(ret));
    REQUIRE(cgkp::is_on_curve(ret));
    REQUIRE(cgkcn::generator_p2_v == ret);
  }

  SECTION("can reproduce doubling results") {
    cgkt::element_p2 a;
    cgkt::element_p2 b;
    cgkt::element_p2 c;

    double_element(a, cgkcn::generator_p2_v); // a = 2g
    double_element(a, a);                     // a = 4g
    double_element(b, cgkcn::generator_p2_v); // b = 2g
    add(c, a, b);                             // c = 6g

    cgkt::element_p2 d{cgkcn::generator_p2_v};
    for (size_t i = 1; i < 6; ++i) {
      add(d, d, cgkcn::generator_p2_v);
    }

    REQUIRE(!cgkp::is_identity(c));
    REQUIRE(cgkp::is_on_curve(c));
    REQUIRE(!cgkp::is_identity(d));
    REQUIRE(cgkp::is_on_curve(d));
    REQUIRE(c == d);
  }

  SECTION("can be done inplace") {
    cgkt::element_p2 lhs{cgkt::element_p2::identity()};
    cgkt::element_p2 rhs{cgkcn::generator_p2_v};

    add_inplace(lhs, rhs);

    REQUIRE(lhs == cgkcn::generator_p2_v);
  }
}

TEST_CASE("addition with mixed elements") {
  SECTION("keeps the identity on the curve") {
    cgkt::element_p2 ret;
    add(ret, cgkt::element_p2::identity(), cgkt::element_affine::identity());

    REQUIRE(cgkp::is_identity(ret));
    REQUIRE(cgkp::is_on_curve(ret));
  }

  SECTION("keeps the generator on the curve") {
    fgkt::element z;
    basn::fast_random_number_generator rng{1, 2};
    fgkrn::generate_random_element(z, rng);

    fgkt::element x;
    fgkt::element y;
    fgko::mul(x, cgkcn::generator_p2_v.X, z);
    fgko::mul(y, cgkcn::generator_p2_v.Y, z);
    const cgkt::element_p2 projected_generator{x, y, z};
    cgkt::element_p2 ret;

    add(ret, projected_generator, cgkt::element_affine::identity());

    REQUIRE(!cgkp::is_identity(ret));
    REQUIRE(cgkp::is_on_curve(ret));
    REQUIRE(cgkcn::generator_p2_v == ret);
  }

  SECTION("can reproduce doubling results") {
    cgkt::element_p2 a;
    cgkt::element_p2 b;
    cgkt::element_p2 c;

    double_element(a, cgkcn::generator_p2_v); // a = 2g
    double_element(a, a);                     // a = 4g
    double_element(b, cgkcn::generator_p2_v); // b = 2g
    add(c, a, b);                             // c = 6g

    cgkt::element_p2 d{cgkcn::generator_p2_v};
    for (size_t i = 1; i < 6; ++i) {
      add(d, d, cgkcn::generator_affine_v);
    }

    REQUIRE(!cgkp::is_identity(d));
    REQUIRE(cgkp::is_on_curve(d));
    REQUIRE(c == d);
  }
}
