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
#include "cbindings/inner_product_proof.h"

#include <vector>

#include "cbindings/backend.h"
#include "sxt/base/error/assert.h"
#include "sxt/base/num/ceil_log2.h"
#include "sxt/base/num/fast_random_number_generator.h"
#include "sxt/base/test/unit_test.h"
#include "sxt/curve21/operation/overload.h"
#include "sxt/curve21/type/element_p3.h"
#include "sxt/proof/transcript/transcript.h"
#include "sxt/ristretto/type/compressed_element.h"
#include "sxt/scalar25/operation/overload.h"
#include "sxt/scalar25/random/element.h"
#include "sxt/scalar25/type/element.h"
#include "sxt/scalar25/type/literal.h"

using namespace sxt;
using namespace sxt::cbn;
using sxt::s25t::operator""_s25;

//--------------------------------------------------------------------------------------------------
// initialize_backend
//--------------------------------------------------------------------------------------------------
static void initialize_backend(int backend, uint64_t precomputed_elements) noexcept {
  const sxt_config config = {backend, precomputed_elements};
  REQUIRE(sxt_init(&config) == 0);
}

//--------------------------------------------------------------------------------------------------
// generate_inner_product_input
//--------------------------------------------------------------------------------------------------
static void generate_inner_product_input(std::vector<s25t::element>& a_vector,
                                         std::vector<s25t::element>& b_vector,
                                         std::vector<c21t::element_p3>& g_vector,
                                         std::vector<rstt::compressed_element>& l_vector,
                                         std::vector<rstt::compressed_element>& r_vector, size_t n,
                                         size_t generators_offset) noexcept {
  a_vector.resize(n);
  b_vector.resize(n);

  basn::fast_random_number_generator rng{1 * n, 2 * n};
  s25rn::generate_random_elements(a_vector, rng);
  s25rn::generate_random_elements(b_vector, rng);

  auto n_lg2 = basn::ceil_log2(n);
  l_vector.resize(n_lg2);
  r_vector.resize(n_lg2);

  auto np = 1ull << n_lg2;
  g_vector.resize(np);

  REQUIRE(sxt_ristretto255_get_generators(reinterpret_cast<sxt_ristretto255*>(g_vector.data()), np,
                                          generators_offset) == 0);
}

//--------------------------------------------------------------------------------------------------
// test_prove_and_verify_with_given_n
//--------------------------------------------------------------------------------------------------
static void test_prove_and_verify_with_given_n(uint64_t n, uint64_t generators_offset) {
  SXT_DEBUG_ASSERT(n > 0);

  s25t::element ap_value;
  prft::transcript transcript{"abc"};
  std::vector<c21t::element_p3> g_vector;
  std::vector<s25t::element> a_vector, b_vector;
  std::vector<rstt::compressed_element> l_vector, r_vector;

  generate_inner_product_input(a_vector, b_vector, g_vector, l_vector, r_vector, n,
                               generators_offset);

  sxt_curve25519_prove_inner_product(
      reinterpret_cast<sxt_ristretto255_compressed*>(l_vector.data()),
      reinterpret_cast<sxt_ristretto255_compressed*>(r_vector.data()),
      reinterpret_cast<sxt_curve25519_scalar*>(&ap_value),
      reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
      reinterpret_cast<const sxt_curve25519_scalar*>(a_vector.data()),
      reinterpret_cast<const sxt_curve25519_scalar*>(b_vector.data()));

  auto product = a_vector[0] * b_vector[0];
  auto a_commit = a_vector[0] * g_vector[0];

  for (size_t i = 1; i < a_vector.size(); ++i) {
    product = product + a_vector[i] * b_vector[i];
    a_commit = a_commit + a_vector[i] * g_vector[i];
  }

  SECTION("We can verify a proof using valid input data") {
    transcript = prft::transcript{"abc"};
    REQUIRE(sxt_curve25519_verify_inner_product(
                reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
                reinterpret_cast<const sxt_curve25519_scalar*>(b_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&product),
                reinterpret_cast<const sxt_ristretto255*>(&a_commit),
                reinterpret_cast<const sxt_ristretto255_compressed*>(l_vector.data()),
                reinterpret_cast<const sxt_ristretto255_compressed*>(r_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&ap_value)) == 1);
  }

  SECTION("We cannot verify a proof using an invalid a_commit") {
    transcript = prft::transcript{"abc"};
    auto a_commit_p = 0x123_s25 * g_vector[0];
    REQUIRE(sxt_curve25519_verify_inner_product(
                reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
                reinterpret_cast<const sxt_curve25519_scalar*>(b_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&product),
                reinterpret_cast<const sxt_ristretto255*>(&a_commit_p),
                reinterpret_cast<const sxt_ristretto255_compressed*>(l_vector.data()),
                reinterpret_cast<const sxt_ristretto255_compressed*>(r_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&ap_value)) == 0);
  }

  SECTION("We cannot verify a proof using an invalid product") {
    transcript = prft::transcript{"abc"};
    auto product_p = product + 0x123_s25;
    REQUIRE(sxt_curve25519_verify_inner_product(
                reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
                reinterpret_cast<const sxt_curve25519_scalar*>(b_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&product_p),
                reinterpret_cast<const sxt_ristretto255*>(&a_commit),
                reinterpret_cast<const sxt_ristretto255_compressed*>(l_vector.data()),
                reinterpret_cast<const sxt_ristretto255_compressed*>(r_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&ap_value)) == 0);
  }

  SECTION("We cannot verify a proof using an invalid b vector") {
    transcript = prft::transcript{"abc"};
    REQUIRE(sxt_curve25519_verify_inner_product(
                reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
                reinterpret_cast<const sxt_curve25519_scalar*>(a_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&product),
                reinterpret_cast<const sxt_ristretto255*>(&a_commit),
                reinterpret_cast<const sxt_ristretto255_compressed*>(l_vector.data()),
                reinterpret_cast<const sxt_ristretto255_compressed*>(r_vector.data()),
                reinterpret_cast<const sxt_curve25519_scalar*>(&ap_value)) == 0);
  }

  // we use the `transcript` only with inputs having at least two elements
  if (n > 1) {
    SECTION("We cannot verify a proof using an invalid transcript") {
      transcript = prft::transcript{"wrong_transcript"};
      REQUIRE(sxt_curve25519_verify_inner_product(
                  reinterpret_cast<sxt_transcript*>(&transcript), n, generators_offset,
                  reinterpret_cast<const sxt_curve25519_scalar*>(a_vector.data()),
                  reinterpret_cast<const sxt_curve25519_scalar*>(&product),
                  reinterpret_cast<const sxt_ristretto255*>(&a_commit),
                  reinterpret_cast<const sxt_ristretto255_compressed*>(l_vector.data()),
                  reinterpret_cast<const sxt_ristretto255_compressed*>(r_vector.data()),
                  reinterpret_cast<const sxt_curve25519_scalar*>(&ap_value)) == 0);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// test_prove_and_verify_with_given_generators_offset
//--------------------------------------------------------------------------------------------------
static void
test_prove_and_verify_with_given_generators_offset(uint64_t generators_offset) noexcept {
  SECTION("We can prove and verify a proof with a single element") {
    uint64_t n = 1;
    test_prove_and_verify_with_given_n(n, generators_offset);
  }

  SECTION("We can prove and verify a proof with 2 elements") {
    uint64_t n = 2;
    test_prove_and_verify_with_given_n(n, generators_offset);
  }

  SECTION("We can prove and verify random proofs of varying size") {
    for (size_t n = 3; n <= 9; ++n) {
      test_prove_and_verify_with_given_n(n, generators_offset);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// test_prove_and_verify_with_specified_precomputed_elements
//--------------------------------------------------------------------------------------------------
static void
test_prove_and_verify_with_specified_precomputed_elements(int backend,
                                                          uint64_t precomputed_elements) {
  initialize_backend(backend, precomputed_elements);

  SECTION("We can prove and verify the inner product with a zero generators offset") {
    uint64_t generators_offset = 0;
    test_prove_and_verify_with_given_generators_offset(generators_offset);
  }

  SECTION("We can prove and verify the inner product with a non-zero generators offset") {
    uint64_t generators_offset = 11;
    test_prove_and_verify_with_given_generators_offset(generators_offset);
  }

  sxt::cbn::reset_backend_for_testing();
}

//--------------------------------------------------------------------------------------------------
// test_prove_and_verify_with_given_backend
//--------------------------------------------------------------------------------------------------
static void test_prove_and_verify_with_given_backend(int backend) {
  SECTION("We can prove and verify without precomputing elements") {
    uint64_t num_precomputed_els = 0;
    test_prove_and_verify_with_specified_precomputed_elements(backend, num_precomputed_els);
  }

  SECTION("We can prove and verify using non-zero precomputed elements") {
    uint64_t num_precomputed_els = 9;
    test_prove_and_verify_with_specified_precomputed_elements(backend, num_precomputed_els);
  }
}

TEST_CASE("We can correctly prove and verify the inner product using the gpu backend") {
  test_prove_and_verify_with_given_backend(SXT_GPU_BACKEND);
}

TEST_CASE("We can correctly prove and verify the inner product using the cpu backend") {
  test_prove_and_verify_with_given_backend(SXT_CPU_BACKEND);
}
