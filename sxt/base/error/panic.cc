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
#include "sxt/base/error/panic.h"

#include <cstdlib>
#include <print>

namespace sxt::baser {
//--------------------------------------------------------------------------------------------------
// panic_with_message
//--------------------------------------------------------------------------------------------------
[[noreturn]] void panic_with_message(std::string_view file, int line, std::string_view msg,
                                     const std::string& trace) noexcept {
  std::print(stderr, "{}:{} panic: {}\n{}\n", file, line, msg, trace);
  std::abort();
}
} // namespace sxt::baser
