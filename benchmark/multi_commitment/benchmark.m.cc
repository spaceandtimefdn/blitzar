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
#include <cuda_profiler_api.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <print>
#include <random>
#include <string>
#include <string_view>

#include "sxt/base/container/span.h"
#include "sxt/base/curve/element.h"
#include "sxt/base/profile/callgrind.h"
#include "sxt/cbindings/backend/computational_backend.h"
#include "sxt/cbindings/backend/cpu_backend.h"
#include "sxt/cbindings/backend/gpu_backend.h"
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
#include "sxt/memory/management/managed_array.h"
#include "sxt/multiexp/base/exponent_sequence.h"
#include "sxt/ristretto/base/byte_conversion.h"
#include "sxt/ristretto/type/compressed_element.h"
#include "sxt/seqcommit/generator/base_element.h"

using namespace sxt;

struct params {
  int status;
  bool verbose;
  int num_samples = 1;
  uint64_t num_commitments;
  uint64_t commitment_length;
  std::string backend_str;
  uint64_t element_nbytes;
  bool is_boolean;
  std::unique_ptr<cbnbck::computational_backend> backend;
  std::string curve;

  std::chrono::steady_clock::time_point begin_time;
  std::chrono::steady_clock::time_point end_time;

  params(int argc, char* argv[]) {
    status = 0;

    if (argc < 7) {
      std::cerr << "Usage: benchmark " << "<cpu|gpu> " << "<curve> " << "<n> " << "<num_samples> "
                << "<num_commitments> " << "<element_nbytes> " << "<verbose>\n";
      status = -1;
    }

    select_backend_fn(argv[1]);
    select_curve(argv[2]);

    verbose = is_boolean = false;
    commitment_length = std::atoi(argv[3]);
    num_samples = std::atoi(argv[4]);
    num_commitments = std::atoi(argv[5]);
    element_nbytes = std::atoi(argv[6]);

    if (num_commitments <= 0 || commitment_length <= 0 || element_nbytes > 32) {
      std::cerr << "Restriction: 1 <= num_commitments, "
                << "1 <= commitment_length, 1 <= element_nbytes <= 32\n";
      status = -1;
    }

    if (element_nbytes == 0) {
      is_boolean = true;
      element_nbytes = 1;
    }

    if (std::atoi(argv[7]) == 1) {
      verbose = true;
    }
  }

  void select_backend_fn(const std::string_view backend_view) noexcept {
    if (backend_view == "gpu") {
      backend_str = "gpu";
      backend = std::make_unique<cbnbck::gpu_backend>();
      return;
    }

    if (backend_view == "cpu") {
      backend_str = "cpu";
      backend = std::make_unique<cbnbck::cpu_backend>();
      return;
    }

    std::cerr << "invalid backend: " << backend_view << "\n";

    status = -1;
  }

  void select_curve(const std::string_view curve_view) noexcept {
    if (curve_view == "curve25519") {
      curve = "curve25519";
      return;
    } else if (curve_view == "bls12_381") {
      curve = "bls12_381";
      return;
    } else if (curve_view == "bn254") {
      curve = "bn254";
      return;
    } else if (curve_view == "grumpkin") {
      curve = "grumpkin";
      return;
    }

    std::cerr << "invalid curve: " << curve_view << "\n";

    status = -1;
  }

  void trigger_timer() { begin_time = std::chrono::steady_clock::now(); }

  void stop_timer() { end_time = std::chrono::steady_clock::now(); }

  double elapsed_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_time - begin_time).count() /
           1e6;
  }
};

//--------------------------------------------------------------------------------------------------
// print_result
//--------------------------------------------------------------------------------------------------
static void print_result(uint64_t num_commitments,
                         memmg::managed_array<rstt::compressed_element>& commitments_per_sequence) {
  // print the 32 bytes commitment results of each sequence
  for (size_t c = 0; c < num_commitments; ++c) {
    std::cout << c << ": " << commitments_per_sequence[c] << std::endl;
  }
}

//--------------------------------------------------------------------------------------------------
// print_result
//--------------------------------------------------------------------------------------------------
static void print_result(uint64_t num_commitments,
                         memmg::managed_array<cg1t::compressed_element> elements) noexcept {
  size_t index = 0;
  for (auto& e : elements) {
    std::cout << index++ << ": " << e << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// print_result
//--------------------------------------------------------------------------------------------------
static void print_result(uint64_t num_commitments,
                         memmg::managed_array<cn1t::element_affine> elements) noexcept {
  size_t index = 0;
  for (auto& e : elements) {
    std::cout << index++ << ": {" << e.X << ", " << e.Y << "}" << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// print_result
//--------------------------------------------------------------------------------------------------
static void print_result(uint64_t num_commitments,
                         memmg::managed_array<cgkt::element_affine> elements) noexcept {
  size_t index = 0;
  for (auto& e : elements) {
    std::cout << index++ << ": {" << e.X << ", " << e.Y << "}" << "\n";
  }
}

//--------------------------------------------------------------------------------------------------
// populate_table
//--------------------------------------------------------------------------------------------------
template <bascrv::element T, typename GeneratorFunc>
static void populate_table(bool is_boolean, uint64_t num_commitments, uint64_t commitment_length,
                           uint8_t element_nbytes, memmg::managed_array<uint8_t>& data_table,
                           memmg::managed_array<mtxb::exponent_sequence>& data_commitments,
                           memmg::managed_array<T>& generators, GeneratorFunc generator_func) {

  std::mt19937 gen{0};
  std::uniform_int_distribution<uint8_t> distribution;

  if (is_boolean) {
    distribution = std::uniform_int_distribution<uint8_t>(0, 1);
  } else {
    distribution = std::uniform_int_distribution<uint8_t>(0, UINT8_MAX);
  }

  for (size_t i = 0; i < commitment_length; ++i) {
    generator_func(generators[i], i);
  }

  for (size_t i = 0; i < data_table.size(); ++i) {
    data_table[i] = distribution(gen);
  }

  for (size_t c = 0; c < num_commitments; ++c) {
    auto& data_sequence = data_commitments[c];
    data_sequence.n = commitment_length;
    data_sequence.element_nbytes = element_nbytes;
    data_sequence.data = data_table.data() + c * commitment_length * element_nbytes;
    data_sequence.is_signed = 0;
  }
}

//--------------------------------------------------------------------------------------------------
// run_benchmark
//--------------------------------------------------------------------------------------------------
template <bascrv::element T, class U, typename GeneratorFunc>
static void run_benchmark(params& p, GeneratorFunc generator_func) {

  memmg::managed_array<uint8_t> data_table(p.commitment_length * p.num_commitments *
                                           p.element_nbytes);

  memmg::managed_array<mtxb::exponent_sequence> data_commitments(p.num_commitments);
  basct::cspan<mtxb::exponent_sequence> value_sequences(data_commitments.data(), p.num_commitments);

  memmg::managed_array<T> generators(p.commitment_length);

  memmg::managed_array<U> commitments_per_sequence(p.num_commitments);
  basct::span<U> commitments(commitments_per_sequence.data(), p.num_commitments);

  populate_table<T>(p.is_boolean, p.num_commitments, p.commitment_length, p.element_nbytes,
                    data_table, data_commitments, generators, generator_func);

  std::vector<double> durations;
  double mean_duration_compute = 0;

  cudaProfilerStart();
  for (int i = 0; i < p.num_samples; ++i) {
    p.trigger_timer();
    SXT_TOGGLE_COLLECT;
    p.backend->compute_commitments(commitments, value_sequences, generators);
    SXT_TOGGLE_COLLECT;
    p.stop_timer();

    double duration_compute = p.elapsed_time();

    durations.push_back(duration_compute);
    mean_duration_compute += duration_compute / p.num_samples;
  }
  cudaProfilerStop();

  double std_deviation = 0;

  for (int i = 0; i < p.num_samples; ++i) {
    std_deviation += pow(durations[i] - mean_duration_compute, 2.);
  }

  std_deviation = sqrt(std_deviation / p.num_samples);

  double data_throughput = p.commitment_length * p.num_commitments / mean_duration_compute;

  std::cout << "compute duration (s) : " << std::fixed << mean_duration_compute << std::endl;
  std::cout << "compute std deviation (s) : " << std::fixed << std_deviation << std::endl;
  std::cout << "throughput (exponentiations / s) : " << std::scientific << data_throughput
            << std::endl;

  if (p.verbose) {
    std::println("===== result");
    print_result(p.num_commitments, commitments_per_sequence);
  }

  std::println("********************************************");
}

//--------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  params p(argc, argv);

  if (p.status != 0) {
    return -1;
  }

  double table_size = (p.num_commitments * p.commitment_length * p.element_nbytes) / 1024.;

  std::println("===== benchmark results");
  std::println("backend : {}", p.backend_str);
  std::println("curve : {}", p.curve);
  std::println("commitment length : {}", p.commitment_length);
  std::println("number of commitments : {}", p.num_commitments);
  std::println("element_nbytes : {}", p.element_nbytes);
  std::println("table_size (MB) : {}", table_size);
  std::println("num_exponentations : {}", (p.num_commitments * p.commitment_length));
  std::println("********************************************");

  if (p.curve == "curve25519") {
    auto generator = [](c21t::element_p3& element, unsigned i) {
      sqcgn::compute_base_element(element, i);
    };

    run_benchmark<c21t::element_p3, rstt::compressed_element>(p, generator);
  } else if (p.curve == "bls12_381") {
    auto generator = [](cg1t::element_p2& element, unsigned i) {
      basn::fast_random_number_generator rng{i + 1, i + 2};
      cg1rn::generate_random_element(element, rng);
    };

    run_benchmark<cg1t::element_p2, cg1t::compressed_element>(p, generator);
  } else if (p.curve == "bn254") {
    auto generator = [](cn1t::element_p2& element, unsigned i) {
      basn::fast_random_number_generator rng{i + 1, i + 2};
      cn1rn::generate_random_element(element, rng);
    };

    run_benchmark<cn1t::element_p2, cn1t::element_affine>(p, generator);
  } else if (p.curve == "grumpkin") {
    auto generator = [](cgkt::element_p2& element, unsigned i) {
      basn::fast_random_number_generator rng{i + 1, i + 2};
      cgkrn::generate_random_element(element, rng);
    };

    run_benchmark<cgkt::element_p2, cgkt::element_affine>(p, generator);
  } else {
    std::cerr << "Unsupported curve: " << p.curve << "\n";
    return -1;
  }

  return 0;
}
