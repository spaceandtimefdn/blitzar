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
#include <numeric>
#include <print>
#include <string_view>

#include "sxt/base/container/span.h"
#include "sxt/base/error/panic.h"
#include "sxt/base/num/ceil_log2.h"
#include "sxt/base/num/fast_random_number_generator.h"
#include "sxt/execution/async/future.h"
#include "sxt/execution/schedule/scheduler.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/proof/sumcheck/chunked_gpu_driver.h"
#include "sxt/proof/sumcheck/cpu_driver.h"
#include "sxt/proof/sumcheck/gpu_driver.h"
#include "sxt/proof/sumcheck/proof_computation.h"
#include "sxt/proof/sumcheck/reference_transcript.h"
#include "sxt/proof/sumcheck/verification.h"
#include "sxt/proof/transcript/transcript.h"
#include "sxt/scalar25/random/element.h"
#include "sxt/scalar25/realization/field.h"

using namespace sxt;

struct params {
  std::string field = "curve25519";
  unsigned n;
  unsigned degree;
  unsigned num_products;
  unsigned num_samples;
};

static bool read_params(params& p, int argc, char* argv[]) noexcept {
  if (argc != 6) {
    std::println("Usage: benchmark <scalar_field> <n> <degree> <num_products> <num_samples>");
    return false;
  }

  std::string_view s;

  // field
  s = {argv[1]};
  if (s == "curve25519") {
    p.field = "curve25519";
  } else {
    baser::panic("invalid scalar field: {}\n", s);
  }

  // n
  s = {argv[2]};
  if (std::from_chars(s.begin(), s.end(), p.n).ec != std::errc{}) {
    baser::panic("invalid argument for n: {}\n", s);
  }

  // degree
  s = {argv[3]};
  if (std::from_chars(s.begin(), s.end(), p.degree).ec != std::errc{}) {
    baser::panic("invalid argument for degree: {}\n", s);
  }

  // num_products
  s = {argv[4]};
  if (std::from_chars(s.begin(), s.end(), p.num_products).ec != std::errc{}) {
    baser::panic("invalid argument for num_products: {}\n", s);
  }

  // num_samples
  s = {argv[5]};
  if (std::from_chars(s.begin(), s.end(), p.num_samples).ec != std::errc{}) {
    baser::panic("invalid argument for num_samples: {}\n", s);
  }

  return true;
}

template <class U>
static void check_verifiy(basct::cspan<U> round_polynomials, unsigned round_degree,
                          unsigned num_rounds) noexcept {
  prft::transcript base_transcript{"abc123"};
  prfsk::reference_transcript<U> transcript{base_transcript};
  memmg::managed_array<U> evaluation_point(num_rounds);
  U expected_sum;
  prfsk::sum_polynomial_01(expected_sum, round_polynomials.subspan(0, round_degree + 1));
  auto was_successful = prfsk::verify_sumcheck_no_evaluation<U>(
      expected_sum, evaluation_point, transcript, round_polynomials, round_degree);
  if (!was_successful) {
    baser::panic("verification failed");
  }
}

template <class U, typename GeneratorFunc>
static void run_benchmark(params& p, GeneratorFunc generator_func) {
  auto num_rounds = basn::ceil_log2(p.n);

  basn::fast_random_number_generator rng{1, 2};

  // mles
  memmg::managed_array<U> mles(p.n * p.degree * p.num_products);
  for (unsigned i = 0; i < mles.size(); ++i) {
    generator_func(mles[i], rng);
  }

  // product_table
  memmg::managed_array<std::pair<U, unsigned>> product_table(p.num_products);
  for (unsigned product_index = 0; product_index < p.num_products; ++product_index) {
    generator_func(product_table[product_index].first, rng);
    product_table[product_index].second = p.degree;
  }

  // product_terms
  memmg::managed_array<unsigned> product_terms(p.num_products * p.degree);
  std::iota(product_terms.begin(), product_terms.end(), 0);

  // benchmark
  memmg::managed_array<U> polynomials((p.degree + 1u) * num_rounds);
  memmg::managed_array<U> evaluation_point(num_rounds);
  prft::transcript base_transcript{"abc123"};
  prfsk::reference_transcript<U> transcript{base_transcript};
  prfsk::chunked_gpu_driver<U> drv;

  // initial run
  {
    auto fut = prfsk::prove_sum<U>(polynomials, evaluation_point, transcript, drv, mles,
                                   product_table, product_terms, p.n);
    xens::get_scheduler().run();
    check_verifiy<U>(polynomials, p.degree, num_rounds);
  }

  // sample
  std::vector<double> durations;
  double mean_duration_compute = 0.0;
  double elapse = 0.0;

  for (unsigned i = 0; i < (p.num_samples + 1u); ++i) {
    auto t1 = std::chrono::steady_clock::now();
    auto fut = prfsk::prove_sum<U>(polynomials, evaluation_point, transcript, drv, mles,
                                   product_table, product_terms, p.n);
    xens::get_scheduler().run();
    auto t2 = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    if (i > 0) {
      auto duration_compute = static_cast<double>(duration.count()) / 1e6;
      durations.push_back(duration_compute);
      elapse += duration_compute;
      mean_duration_compute += duration_compute / p.num_samples;
    }
  }

  double std_deviation = 0.0;

  for (int i = 0; i < p.num_samples; ++i) {
    std_deviation += pow(durations[i] - mean_duration_compute, 2.);
  }

  std_deviation = sqrt(std_deviation / p.num_samples);

  std::println("compute duration (s): {:.4e}", elapse / p.num_samples);
  std::println("compute std deviation (s): {:.4e}", std_deviation);
  std::println("throughput (s): {:.4e}", p.n / (elapse / p.num_samples));
  std::println("********************************************");
}

int main(int argc, char* argv[]) {
  params p;
  if (!read_params(p, argc, argv)) {
    return -1;
  }

  auto num_rounds = basn::ceil_log2(p.n);
  std::println("===== benchmark results");
  std::println("scalar field: {}", p.field);
  std::println("n = {}", p.n);
  std::println("num_rounds = {}", num_rounds);
  std::println("degree = {}", p.degree);
  std::println("num_products = {}", p.num_products);
  std::println("num_samples = {}", p.num_samples);
  std::println("********************************************");

  if (p.field == "curve25519") {
    auto generator = [](s25t::element& element, basn::fast_random_number_generator& rng) {
      s25rn::generate_random_element(element, rng);
    };
    run_benchmark<s25t::element>(p, generator);
  } else {
    baser::panic("unsupported scalar field: {}", p.field);
  }

  return 0;
}
