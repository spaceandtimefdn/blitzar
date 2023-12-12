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
#include "sxt/proof/inner_product/verification_kernel.h"

#include <limits>

#include "sxt/algorithm/iteration/for_each.h"
#include "sxt/algorithm/iteration/transform.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/error/assert.h"
#include "sxt/execution/async/future.h"
#include "sxt/execution/device/synchronization.h"
#include "sxt/scalar25/operation/mul.h"
#include "sxt/scalar25/type/element.h"

namespace sxt::prfip {
//--------------------------------------------------------------------------------------------------
// compute_g_exponents_partial
//--------------------------------------------------------------------------------------------------
xena::future<> async_compute_g_exponents_partial(basct::span<s25t::element> g_exponents,
                                                 basct::cspan<s25t::element> x_sq_vector,
                                                 size_t round_first) noexcept {
  auto num_rounds = round_first - 1 + x_sq_vector.size();
  auto np = 1ull << num_rounds;
  // clang-format off
  SXT_DEBUG_ASSERT(
      np <= std::numeric_limits<unsigned>::max() &&
      basdv::is_host_pointer(g_exponents.data()) &&
      basdv::is_host_pointer(x_sq_vector.data()) &&
      g_exponents.size() == np &&
      !x_sq_vector.empty() &&
      round_first > 0
  );
  // clang-format on
  auto a = static_cast<unsigned>(1ull << (round_first - 1));
  auto multiplier_iter = x_sq_vector.data() + x_sq_vector.size();
  // Note: These haven't been informed by much benchmarking. I'm
  // sure there are better values. This is just putting in some
  // ballpark estimates to get started.
  basit::chunk_options chunk_options{
      .min_size = 1u << 10u,
      .max_size = 1u << 20u,
  };
  while (a != np) {
    auto& multiplier = *--multiplier_iter;
    auto f = [multiplier] __device__ __host__(s25t::element & val) noexcept {
      s25o::mul(val, multiplier, val);
    };
    co_await algi::transform(g_exponents.subspan(a, a), chunk_options, f,
                             g_exponents.subspan(0, a));
    a *= 2u;
  }
}
} // namespace sxt::prfip
