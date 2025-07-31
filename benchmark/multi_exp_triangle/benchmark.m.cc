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
#include <charconv>
#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <print>
#include <random>
#include <string_view>
#include <type_traits>

#include "sxt/base/curve/element.h"
#include "sxt/base/num/fast_random_number_generator.h"
#include "sxt/curve21/operation/add.h"
#include "sxt/curve21/operation/double.h"
#include "sxt/curve21/operation/neg.h"
#include "sxt/curve21/type/element_p3.h"
#include "sxt/curve_bng1/operation/add.h"
#include "sxt/curve_bng1/operation/double.h"
#include "sxt/curve_bng1/operation/neg.h"
#include "sxt/curve_bng1/random/element_p2.h"
#include "sxt/curve_bng1/type/conversion_utility.h"
#include "sxt/curve_bng1/type/element_affine.h"
#include "sxt/curve_bng1/type/element_p2.h"
#include "sxt/curve_g1/operation/add.h"
#include "sxt/curve_g1/operation/compression.h"
#include "sxt/curve_g1/operation/double.h"
#include "sxt/curve_g1/operation/neg.h"
#include "sxt/curve_g1/random/element_p2.h"
#include "sxt/curve_g1/type/compressed_element.h"
#include "sxt/curve_g1/type/conversion_utility.h"
#include "sxt/curve_g1/type/element_p2.h"
#include "sxt/curve_gk/operation/add.h"
#include "sxt/curve_gk/operation/double.h"
#include "sxt/curve_gk/operation/neg.h"
#include "sxt/curve_gk/random/element_p2.h"
#include "sxt/curve_gk/type/conversion_utility.h"
#include "sxt/curve_gk/type/element_affine.h"
#include "sxt/curve_gk/type/element_p2.h"
#include "sxt/execution/schedule/scheduler.h"
#include "sxt/memory/resource/pinned_resource.h"
#include "sxt/multiexp/pippenger2/in_memory_partition_table_accessor_utility.h"
#include "sxt/multiexp/pippenger2/variable_length_multiexponentiation.h"
#include "sxt/ristretto/operation/compression.h"
#include "sxt/ristretto/type/compressed_element.h"
#include "sxt/seqcommit/generator/base_element.h"

using namespace sxt;

//--------------------------------------------------------------------------------------------------
// make_partition_table_accessor
//--------------------------------------------------------------------------------------------------
template <class U, typename T, typename GeneratorFunc>
static std::unique_ptr<mtxpp2::partition_table_accessor<U>>
make_partition_table_accessor(unsigned n, GeneratorFunc generatorFunc) noexcept {
  std::vector<T> generators(n);
  for (unsigned i = 0; i < n; ++i) {
    generatorFunc(generators[i], i);
  }
  return mtxpp2::make_in_memory_partition_table_accessor<U, T>(generators);
}

//--------------------------------------------------------------------------------------------------
// curve25519_generator
//--------------------------------------------------------------------------------------------------
static void curve25519_generator(c21t::element_p3& element, unsigned i) {
  sqcgn::compute_base_element(element, i);
}

//--------------------------------------------------------------------------------------------------
// bls12_381_generator
//--------------------------------------------------------------------------------------------------
static void bls12_381_generator(cg1t::element_p2& element, unsigned i) {
  basn::fast_random_number_generator rng{i + 1, i + 2};
  cg1rn::generate_random_element(element, rng);
}

//--------------------------------------------------------------------------------------------------
// bn254_generator
//--------------------------------------------------------------------------------------------------
static void bn254_generator(cn1t::element_p2& element, unsigned i) {
  basn::fast_random_number_generator rng{i + 1, i + 2};
  cn1rn::generate_random_element(element, rng);
}

//--------------------------------------------------------------------------------------------------
// grumpkin_generator
//--------------------------------------------------------------------------------------------------
static void grumpkin_generator(cgkt::element_p2& element, unsigned i) {
  basn::fast_random_number_generator rng{i + 1, i + 2};
  cgkrn::generate_random_element(element, rng);
}

//--------------------------------------------------------------------------------------------------
// fill_exponents
//--------------------------------------------------------------------------------------------------
static void fill_exponents(memmg::managed_array<uint8_t>& exponents, unsigned element_num_bytes,
                           unsigned num_outputs, unsigned n) noexcept {
  exponents.resize(num_outputs * n * element_num_bytes);
  std::mt19937 rng{0};
  std::uniform_int_distribution<uint8_t> dist{0, std::numeric_limits<uint8_t>::max()};
  for (unsigned output_index = 0; output_index < num_outputs; ++output_index) {
    for (unsigned i = 0; i < n; ++i) {
      for (unsigned byte_index = 0; byte_index < element_num_bytes; ++byte_index) {
        exponents[byte_index + element_num_bytes * output_index +
                  element_num_bytes * num_outputs * i] = dist(rng);
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------
// print_elements
//--------------------------------------------------------------------------------------------------
static void print_elements(basct::cspan<c21t::element_p3> elements) noexcept {
  rstt::compressed_element r;
  size_t index = 0;
  for (auto& e : elements) {
    rsto::compress(r, e);
    std::cout << index++ << ": " << r << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// print_elements
//--------------------------------------------------------------------------------------------------
static void print_elements(basct::cspan<cg1t::element_p2> elements) noexcept {
  cg1t::compressed_element e_c;
  size_t index = 0;
  for (auto& e : elements) {
    cg1o::compress(e_c, e);
    std::cout << index++ << ": " << e_c << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// print_elements
//--------------------------------------------------------------------------------------------------
static void print_elements(basct::cspan<cn1t::element_p2> elements) noexcept {
  size_t index = 0;
  for (auto& e : elements) {
    cn1t::element_affine e_affine;
    cn1t::to_element_affine(e_affine, e);
    std::cout << index++ << ": {" << e_affine.X << ", " << e_affine.Y << "}" << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// print_elements
//--------------------------------------------------------------------------------------------------
static void print_elements(basct::cspan<cgkt::element_p2> elements) noexcept {
  size_t index = 0;
  for (auto& e : elements) {
    cgkt::element_affine e_affine;
    cgkt::to_element_affine(e_affine, e);
    std::cout << index++ << ": {" << e_affine.X << ", " << e_affine.Y << "}" << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// run_benchmark
//--------------------------------------------------------------------------------------------------
template <bascrv::element T, class U>
static void run_benchmark(std::unique_ptr<mtxpp2::partition_table_accessor<U>>& accessor,
                          unsigned num_samples, unsigned num_outputs, unsigned element_num_bytes,
                          unsigned n, bool verbose) {

  memmg::managed_array<uint8_t> exponents;
  fill_exponents(exponents, element_num_bytes, num_outputs, n);

  memmg::managed_array<T> res{num_outputs, memr::get_pinned_resource()};

  memmg::managed_array<unsigned> output_bit_widths(num_outputs);
  for (unsigned i = 0; i < num_outputs; ++i) {
    output_bit_widths[i] = element_num_bytes * 8;
  }

  memmg::managed_array<unsigned> output_lengths(num_outputs);
  auto length_counter = [&]() noexcept {
    if (n > num_outputs) {
      return n - num_outputs;
    } else {
      return 0u;
    }
  }();
  for (unsigned i = 0; i < num_outputs; ++i) {
    output_lengths[i] = std::min(++length_counter, n);
  }

  // discard initial run
  {
    auto fut = mtxpp2::async_multiexponentiate<T>(res, *accessor, output_bit_widths, output_lengths,
                                                  exponents);
    xens::get_scheduler().run();
  }

  // run benchmark
  std::vector<double> durations;
  double mean_duration_compute = 0.0;

  for (unsigned i = 0; i < num_samples; ++i) {
    auto t1 = std::chrono::steady_clock::now();

    auto fut = mtxpp2::async_multiexponentiate<T>(res, *accessor, output_bit_widths, output_lengths,
                                                  exponents);
    xens::get_scheduler().run();

    auto t2 = std::chrono::steady_clock::now();

    auto duration_compute =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1e6;
    durations.push_back(duration_compute);
    mean_duration_compute += duration_compute / num_samples;
  }

  double std_deviation = 0;

  for (int i = 0; i < num_samples; ++i) {
    std_deviation += pow(durations[i] - mean_duration_compute, 2.);
  }

  std_deviation = sqrt(std_deviation / num_samples);

  double data_throughput = n * num_outputs / mean_duration_compute;

  std::cout << "compute duration (s) : " << std::fixed << mean_duration_compute << std::endl;
  std::cout << "compute std deviation (s) : " << std::fixed << std_deviation << std::endl;
  std::cout << "throughput (exponentiations / s) : " << std::scientific << data_throughput
            << std::endl;

  if (verbose) {
    std::println("===== result");
    print_elements(res);
  }

  std::println("********************************************");
}

//--------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc != 7) {
    std::println(
        "Usage: benchmark <curve> <n> <num_samples> <num_outputs> <element_nbytes> <verbose>");
    return -1;
  }

  // read arguments
  std::string_view curve_str{argv[1]};
  std::string_view n_str{argv[2]};
  std::string_view num_samples_str{argv[3]};
  std::string_view num_outputs_str{argv[4]};
  std::string_view element_num_bytes_str{argv[5]};
  bool verbose = std::string_view{argv[6]} != "0";
  unsigned n, num_samples, num_outputs, element_num_bytes;
  if (std::from_chars(n_str.begin(), n_str.end(), n).ec != std::errc{}) {
    std::println("invalid argument: {}\n", n_str);
    return -1;
  }
  if (std::from_chars(num_samples_str.begin(), num_samples_str.end(), num_samples).ec !=
      std::errc{}) {
    std::println("invalid argument: {}\n", num_samples_str);
    return -1;
  }
  if (std::from_chars(num_outputs_str.begin(), num_outputs_str.end(), num_outputs).ec !=
      std::errc{}) {
    std::println("invalid argument: {}\n", num_outputs_str);
    return -1;
  }
  if (std::from_chars(element_num_bytes_str.begin(), element_num_bytes_str.end(), element_num_bytes)
          .ec != std::errc{}) {
    std::println("invalid argument: {}\n", element_num_bytes_str);
    return -1;
  }

  // set up data
  double table_size = (num_outputs * n * element_num_bytes) / 1024.;

  std::println("===== benchmark results");
  std::println("backend : gpu");
  std::println("curve : {}", curve_str);
  std::println("commitment length : {}", n);
  std::println("number of commitments : {}", num_outputs);
  std::println("element_nbytes : {}", element_num_bytes);
  std::println("table_size (MB) : {}", table_size);
  std::println("num_exponentations : {}", (num_outputs * n));
  std::println("********************************************");

  if (curve_str == "curve25519") {
    auto accessor = make_partition_table_accessor<c21t::compact_element, c21t::element_p3>(
        n, curve25519_generator);
    run_benchmark<c21t::element_p3>(accessor, num_samples, num_outputs, element_num_bytes, n,
                                    verbose);
  } else if (curve_str == "bls12_381" || curve_str == "bls12-381") {
    auto accessor = make_partition_table_accessor<cg1t::compact_element, cg1t::element_p2>(
        n, bls12_381_generator);
    run_benchmark<cg1t::element_p2>(accessor, num_samples, num_outputs, element_num_bytes, n,
                                    verbose);
  } else if (curve_str == "bn254") {
    auto accessor =
        make_partition_table_accessor<cn1t::compact_element, cn1t::element_p2>(n, bn254_generator);
    run_benchmark<cn1t::element_p2>(accessor, num_samples, num_outputs, element_num_bytes, n,
                                    verbose);
  } else if (curve_str == "grumpkin") {
    auto accessor = make_partition_table_accessor<cgkt::compact_element, cgkt::element_p2>(
        n, grumpkin_generator);
    run_benchmark<cgkt::element_p2>(accessor, num_samples, num_outputs, element_num_bytes, n,
                                    verbose);
  } else {
    std::println("curve not supported");
  }

  return 0;
}
