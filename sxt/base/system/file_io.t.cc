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
#include "sxt/base/system/file_io.h"

#include <vector>

#include "sxt/base/test/temp_file.h"
#include "sxt/base/test/unit_test.h"

using namespace sxt;
using namespace sxt::bassy;

TEST_CASE("we can read and write data to files") {
  bastst::temp_file out;
  out.stream().close();

  std::vector<int> vals = {1, 2, 3};

  SECTION("we can write values and then read them back") {
    write_file<int>(out.name().c_str(), vals);
    std::vector<int> vals_p;
    read_file<int>(vals_p, out.name().c_str());
    REQUIRE(vals == vals_p);
  }
}
