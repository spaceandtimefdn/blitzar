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
/**
 * Adopted from zkcrypto/bls12_381
 *
 * Copyright (c) 2021
 * Sean Bowe <ewillbefull@gmail.com>
 * Jack Grigg <thestr4d@gmail.com>
 *
 * See third_party/license/zkcrypto.LICENSE
 */
#include "sxt/curve_gk/operation/neg.h"

#include "sxt/curve_gk/type/element_p2.h"
#include "sxt/fieldgk/operation/cmov.h"
#include "sxt/fieldgk/operation/neg.h"
#include "sxt/fieldgk/type/element.h"

namespace sxt::cgko {
//--------------------------------------------------------------------------------------------------
// neg
//--------------------------------------------------------------------------------------------------
CUDA_CALLABLE
void neg(cgkt::element_p2& r, const cgkt::element_p2& p) noexcept {
  r.X = p.X;
  fgko::neg(r.Y, p.Y);
  r.Z = p.Z;
}

//--------------------------------------------------------------------------------------------------
// cneg
//--------------------------------------------------------------------------------------------------
CUDA_CALLABLE
void cneg(cgkt::element_p2& r, unsigned int b) noexcept {
  fgkt::element t;
  fgko::neg(t, r.Y);
  fgko::cmov(r.Y, t, b);
}
} // namespace sxt::cgko
