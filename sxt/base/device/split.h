/** Proofs GPU - Space and Time's cryptographic proof algorithms on the CPU and GPU.
 *
 * Copyright 2025-present Space and Time Labs, Inc.
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

#include "sxt/base/iterator/split.h"

namespace sxt::basdv {
//--------------------------------------------------------------------------------------------------
// plan_split_impl
//--------------------------------------------------------------------------------------------------
basit::split_options plan_split_impl(size_t bytes, size_t total_device_memory,
                                     double memory_target_low, double memory_target_high,
                                     size_t split_factor) noexcept;

//--------------------------------------------------------------------------------------------------
// plan_split
//--------------------------------------------------------------------------------------------------
basit::split_options plan_split(size_t bytes) noexcept;
} // namespace sxt::basdv
