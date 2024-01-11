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
#include "sxt/field25/random/element.h"

#include "sxt/base/macro/cuda_warning.h"
#include "sxt/base/num/fast_random_number_generator.h"
#include "sxt/field25/base/byte_conversion.h"
#include "sxt/field25/type/element.h"

namespace sxt::f25rn {
//--------------------------------------------------------------------------------------------------
// generate_random_element_impl
//--------------------------------------------------------------------------------------------------
CUDA_DISABLE_HOSTDEV_WARNING
template <class Rng>
static CUDA_CALLABLE void generate_random_element_impl(f25t::element& e, Rng& generator) noexcept {
  uint64_t data[4];
  for (int i = 0; i < 4; ++i) {
    data[i] = generator();
  }

  bool is_below_modulus{false};
  f25b::from_bytes(is_below_modulus, e.data(), reinterpret_cast<const uint8_t*>(data));
}

//--------------------------------------------------------------------------------------------------
// generate_random_element
//--------------------------------------------------------------------------------------------------
CUDA_CALLABLE
void generate_random_element(f25t::element& e, basn::fast_random_number_generator& rng) noexcept {
  generate_random_element_impl(e, rng);
}

void generate_random_element(f25t::element& e, std::mt19937& rng) noexcept {
  generate_random_element_impl(e, rng);
}
} // namespace sxt::f25rn
