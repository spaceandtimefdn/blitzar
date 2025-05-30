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
#include "sxt/multiexp/bucket_method/fold_kernel.h"

#include "sxt/base/curve/example_element.h"
#include "sxt/base/device/synchronization.h"
#include "sxt/base/test/unit_test.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/managed_device_resource.h"

using namespace sxt;
using namespace sxt::mtxbk;

TEST_CASE("we can fold partial bucket sums") {
  memmg::managed_array<bascrv::element97> partial_bucket_sums(memr::get_managed_device_resource());
  memmg::managed_array<bascrv::element97> bucket_sums(memr::get_managed_device_resource());

  SECTION("with a single bucket") {
    partial_bucket_sums = {1u};
    bucket_sums = {0u};
    segmented_left_fold_partial_bucket_sums<<<1, 1>>>(bucket_sums.data(),
                                                      partial_bucket_sums.data(), 1, 1);
    basdv::synchronize_device();
    REQUIRE(bucket_sums[0] == 1u);
  }

  SECTION("with multiple buckets") {
    partial_bucket_sums = {1u, 2u, 3u};
    bucket_sums = {0u};
    segmented_left_fold_partial_bucket_sums<<<1, 1>>>(bucket_sums.data(),
                                                      partial_bucket_sums.data(), 1, 3);
    basdv::synchronize_device();
    REQUIRE(bucket_sums[0] == 1u + 2u + 3u);
  }

  SECTION("into multiple buckets") {
    partial_bucket_sums = {1u, 2u, 3u, 4u, 5u, 6u};
    bucket_sums = {0u, 0u, 0u};
    segmented_left_fold_partial_bucket_sums<<<3, 1>>>(bucket_sums.data(),
                                                      partial_bucket_sums.data(), 3, 6);
    basdv::synchronize_device();
    REQUIRE(bucket_sums[0] == 1u + 4u);
    REQUIRE(bucket_sums[1] == 2u + 5u);
    REQUIRE(bucket_sums[2] == 3u + 6u);
  }
}
