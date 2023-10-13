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
#include <cstddef>
#include <iostream>

#include "cbindings/blitzar_api.h"

int main() {
  const sxt_config config = {SXT_GPU_BACKEND};

  if (sxt_init(&config) != 0) {
    std::cerr << "sxt_init failed\n";
    return -1;
  }
  const uint64_t n1 = 3;
  const uint8_t n1_num_bytes = 1;
  uint8_t data_bytes_1[n1_num_bytes * n1] = {1, 2, 3};
  sxt_sequence_descriptor descriptor1 = {
      n1_num_bytes, // number bytes
      n1,           // number rows
      data_bytes_1, // data pointer
      0,            // elements are not signed
  };
  const int num_sequences = 1;
  const sxt_sequence_descriptor descriptors[num_sequences] = {descriptor1};
  sxt_ristretto255_compressed commitments[num_sequences];
  sxt_curve25519_compute_pedersen_commitments(commitments, num_sequences, descriptors, 0);
  auto commitments_data = reinterpret_cast<unsigned char*>(commitments);
  for (size_t i = 0; i < sizeof(commitments); ++i) {
    std::cout << std::hex << static_cast<unsigned>(commitments_data[i]);
  }
  std::cout << "\n";
  return 0;
}
