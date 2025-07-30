# Multi Commitment
Computes a multi commitment using Curve25519 elements.

## Running the benchmark
```
bazel run -c opt //benchmark/multi_commitment:benchmark <cpu|gpu> <n> <num_samples> <num_commitments> <element_nbytes> <verbose>
```
