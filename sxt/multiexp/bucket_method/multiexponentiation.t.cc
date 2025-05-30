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
#include "sxt/multiexp/bucket_method/multiexponentiation.h"

#include "sxt/base/curve/example_element.h"
#include "sxt/base/device/synchronization.h"
#include "sxt/base/test/unit_test.h"
#include "sxt/curve21/operation/add.h"
#include "sxt/curve21/operation/double.h"
#include "sxt/curve21/operation/neg.h"
#include "sxt/curve21/type/element_p3.h"
#include "sxt/curve21/type/literal.h"
#include "sxt/execution/schedule/scheduler.h"
#include "sxt/field51/type/literal.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/managed_device_resource.h"

using namespace sxt;
using namespace sxt::mtxbk;
using c21t::operator""_c21;
using f51t::operator""_f51;

TEST_CASE("we can compute a multiexponentiation") {
  std::vector<bascrv::element97> res(1);
  std::vector<bascrv::element97> generators;
  std::vector<const uint8_t*> exponents;

  SECTION("we can compute a multiexponentiation with no elements") {
    res.clear();
    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    REQUIRE(fut.ready());
  }

  SECTION("we can compute a multiexponentiation with a single zero element") {
    uint8_t scalar_data[32] = {};
    exponents.push_back(scalar_data);
    generators = {12u};
    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 0u);
  }

  SECTION("we can compute a multiexponentiation with a single element of 1") {
    uint8_t scalar_data[32] = {};
    scalar_data[0] = 1;
    exponents.push_back(scalar_data);
    generators = {12u};
    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 12u);
  }

  SECTION("we can compute a multiexponentiation with a single element of 2") {
    uint8_t scalar_data[32] = {};
    scalar_data[0] = 2;
    exponents.push_back(scalar_data);
    generators = {12u};
    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 24u);
  }

  SECTION("we can compute a multiexponentiation with a single element of 256") {
    uint8_t scalar_data[32] = {};
    scalar_data[1] = 1;
    exponents.push_back(scalar_data);
    generators = {12u};
    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 256u * 12u);
  }

  SECTION("we can compute a multiexponentiation with 2 generators") {
    uint8_t scalar_data[64] = {};
    scalar_data[0] = 2;
    scalar_data[32] = 3;
    exponents.push_back(scalar_data);

    generators = {12u, 34u};

    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 2u * 12u + 3u * 34u);
  }

  SECTION("we can compute a multiexponetiation with multiple outputs") {
    res.resize(2);

    uint8_t scalar_data1[64] = {};
    scalar_data1[0] = 2;
    scalar_data1[32] = 3;
    exponents.push_back(scalar_data1);

    uint8_t scalar_data2[64] = {};
    scalar_data2[0] = 7;
    scalar_data2[32] = 4;
    exponents.push_back(scalar_data2);

    generators = {12u, 34u};

    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 2u * 12u + 3u * 34u);
    REQUIRE(res[1] == 7u * 12u + 4u * 34u);
  }
}

TEST_CASE("we can compute multiexponentiations with curve-21") {
  std::vector<c21t::element_p3> res(1);
  std::vector<c21t::element_p3> generators;
  std::vector<const uint8_t*> exponents;

  SECTION("we can compute a multiexponentiation with a single element of 1") {
    uint8_t scalar_data[32] = {};
    scalar_data[0] = 1;
    exponents.push_back(scalar_data);
    generators = {0x123_c21};
    auto fut = multiexponentiate<c21t::element_p3>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(res[0] == 0x123_c21);
  }
}

TEST_CASE("we can compute multiexponentiations with exponent sequences") {
  std::vector<mtxb::exponent_sequence> exponents;
  size_t min_length = 0;

  SECTION("exponentiation fails if the exponent sequences are not 32 bytes") {
    exponents.push_back({
        .element_nbytes = 1,
        .n = 1,
    });
    std::vector<bascrv::element97> generators = {2u};
    auto fut = try_multiexponentiate<bascrv::element97>(generators, exponents, min_length);
    REQUIRE(fut.ready());
    REQUIRE(fut.value().empty());
  }

  SECTION("exponentiation fails if the exponent sequences are different lengths") {
    exponents.push_back({
        .element_nbytes = 32,
        .n = 1,
    });
    exponents.push_back({
        .element_nbytes = 32,
        .n = 2,
    });
    std::vector<bascrv::element97> generators = {2u, 5u};
    auto fut = try_multiexponentiate<bascrv::element97>(generators, exponents, min_length);
    REQUIRE(fut.ready());
    REQUIRE(fut.value().empty());
  }

  SECTION("exponentiation fails if the exponent sequences are too small") {
    exponents.push_back({
        .element_nbytes = 32,
        .n = 1,
    });
    std::vector<bascrv::element97> generators = {2u};
    auto fut = try_multiexponentiate<bascrv::element97>(generators, exponents, 2);
    REQUIRE(fut.ready());
    REQUIRE(fut.value().empty());
  }

  SECTION("exponentiation succeeds if preconditions are met") {
    uint8_t scalar_data[32] = {};
    scalar_data[0] = 12;
    exponents.push_back({
        .element_nbytes = 32,
        .n = 1,
        .data = scalar_data,
    });
    std::vector<bascrv::element97> generators = {2u};
    auto fut = try_multiexponentiate<bascrv::element97>(generators, exponents, min_length);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());
    REQUIRE(fut.value().size() == 1);
    REQUIRE(fut.value()[0] == 2u * 12u);
  }
}

// Max chunk size in mtxbk::accumulate_buckets_impl is currently 1<<20.
namespace {
constexpr size_t max_chunk_size = 1 << 20;
}

TEST_CASE("we can compute a multiexponentiation with elements over max chunk size") {
  const size_t num_outputs = 1;
  std::array<size_t, 2> num_elements_array = {max_chunk_size + 1, max_chunk_size * 2 + 1};

  std::vector<const uint8_t*> exponents;

  for (const auto& num_elements : num_elements_array) {
    std::vector<bascrv::element97> generators(num_elements, 1u);

    std::vector<uint8_t> scalar_data(num_elements * 32, 0);
    for (size_t i = 0; i < num_elements; ++i) {
      scalar_data[i * 32] = 1;
    }

    std::vector<bascrv::element97> res(num_outputs);

    exponents.clear();
    for (size_t i = 0; i < num_outputs; ++i) {
      exponents.push_back(scalar_data.data());
    }

    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());

    const auto expected = bascrv::element97(num_elements);
    for (size_t i = 0; i < num_outputs; ++i) {
      REQUIRE(res[i] == expected);
    }
  }
}

TEST_CASE("we can compute a multiexponentiation with multiple outputs over max chunk size") {
  // num_outputs_array + num_elements is chosen to ensure that the total number of elements exceeds
  // the max_chunk_size.
  std::array<size_t, 4> num_outputs_array = {1 << 3, 1 << 4, 1 << 5, 1 << 6};
  const size_t num_elements = (1 << 17) + 1;

  std::vector<const uint8_t*> exponents;
  std::vector<bascrv::element97> generators(num_elements, 1u);
  std::vector<uint8_t> scalar_data(num_elements * 32, 0);
  for (size_t i = 0; i < num_elements; ++i) {
    scalar_data[i * 32] = 1;
  }

  for (const auto& num_outputs : num_outputs_array) {
    std::vector<bascrv::element97> res(num_outputs);

    exponents.clear();
    for (size_t i = 0; i < num_outputs; ++i) {
      exponents.push_back(scalar_data.data());
    }

    auto fut = multiexponentiate<bascrv::element97>(res, generators, exponents);
    xens::get_scheduler().run();
    REQUIRE(fut.ready());

    const auto expected = bascrv::element97(num_elements);
    for (size_t i = 0; i < num_outputs; ++i) {
      REQUIRE(res[i] == expected);
    }
  }
}
