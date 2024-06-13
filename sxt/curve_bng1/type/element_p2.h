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
#pragma once

#include "sxt/base/macro/cuda_callable.h"
#include "sxt/curve_bng1/type/compact_element.h"
#include "sxt/curve_bng1/type/operation_adl_stub.h"
#include "sxt/field25/constant/one.h"
#include "sxt/field25/constant/zero.h"
#include "sxt/field25/operation/cmov.h"
#include "sxt/field25/type/element.h"

namespace sxt::cn1t {
//--------------------------------------------------------------------------------------------------
// element_p2
//--------------------------------------------------------------------------------------------------
/**
 * Projective coordinates (X,Y,Z). Represents the Affine coordinate point (X/Z,Y/Z).
 * Homogeneous form Y^2 * Z = X^3 + (4 * Z^3).
 */
struct element_p2 : cn1o::operation_adl_stub {
  element_p2() noexcept = default;

  constexpr element_p2(const f25t::element& X, const f25t::element& Y,
                       const f25t::element& Z) noexcept
      : X{X}, Y{Y}, Z{Z} {}

  CUDA_CALLABLE explicit element_p2(const compact_element& e) noexcept
      : X{e.X}, Y{e.Y}, Z{f25cn::one_v} {
    auto is_identity = e.is_identity();
    f25o::cmov(X, f25cn::zero_v, is_identity);
    f25o::cmov(Z, f25cn::zero_v, is_identity);
  }

  CUDA_CALLABLE explicit operator compact_element() const noexcept;

  f25t::element X;
  f25t::element Y;
  f25t::element Z;

  static constexpr element_p2 identity() noexcept {
    return element_p2{f25cn::zero_v, f25cn::one_v, f25cn::zero_v};
  }
};

//--------------------------------------------------------------------------------------------------
// mark
//--------------------------------------------------------------------------------------------------
void mark(element_p2& e) noexcept;

//--------------------------------------------------------------------------------------------------
// is_marked
//--------------------------------------------------------------------------------------------------
bool is_marked(const element_p2& e) noexcept;

//--------------------------------------------------------------------------------------------------
// operator==
//--------------------------------------------------------------------------------------------------
bool operator==(const element_p2& lhs, const element_p2& rhs) noexcept;
} // namespace sxt::cn1t
