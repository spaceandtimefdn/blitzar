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
/*
 * Adopted from zcash/librustzcash
 *
 * Copyright (c) 2017
 * Zcash Company
 *
 * See third_party/license/zcash.LICENSE
 */
#pragma once

#include "sxt/base/macro/cuda_callable.h"
#include "sxt/base/num/cmov.h"
#include "sxt/curve_g1/type/element_affine.h"
#include "sxt/curve_g1/type/element_p2.h"
#include "sxt/field12/operation/cmov.h"
#include "sxt/field12/operation/invert.h"
#include "sxt/field12/operation/mul.h"
#include "sxt/field12/type/element.h"

namespace sxt::cg1t {
//--------------------------------------------------------------------------------------------------
// to_element_affine
//--------------------------------------------------------------------------------------------------
/*
 Converts projective to affine element.
 */
CUDA_CALLABLE
inline void to_element_affine(element_affine& a, const element_p2& p) noexcept {
  f12t::element z_inv;
  const bool is_zero{f12o::invert(z_inv, p.Z)};
  f12o::cmov(z_inv, f12cn::zero_v, is_zero);

  f12t::element x;
  f12t::element y;
  f12o::mul(x, p.X, z_inv);
  f12o::mul(y, p.Y, z_inv);

  a.X = x;
  a.Y = y;
  a.infinity = false;

  f12o::cmov(a.X, element_affine::identity().X, is_zero);
  f12o::cmov(a.Y, element_affine::identity().Y, is_zero);
  basn::cmov(a.infinity, element_affine::identity().infinity, is_zero);
}

//--------------------------------------------------------------------------------------------------
// to_element_p2
//--------------------------------------------------------------------------------------------------
/*
 Converts affine to projective element.
 */
CUDA_CALLABLE
inline void to_element_p2(element_p2& p, const element_affine& a) noexcept {
  p.X = a.X;
  p.Y = a.Y;
  p.Z = f12cn::one_v;
  f12o::cmov(p.Z, f12cn::zero_v, a.infinity);
}
} // namespace sxt::cg1t
