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

#include "sxt/fieldgk/type/element.h"

namespace sxt::cgkcn {
//--------------------------------------------------------------------------------------------------
// b_v
//--------------------------------------------------------------------------------------------------
/**
 * b_v is -17 in Montgomery form.
 * Used in the Grumpkin curve equation: y^2 = x^3 - 17
 */
static constexpr fgkt::element b_v{0xdd7056026000005a, 0x223fa97acb319311, 0xcc388229877910c0,
                                   0x34394632b724eaa};
} // namespace sxt::cgkcn
