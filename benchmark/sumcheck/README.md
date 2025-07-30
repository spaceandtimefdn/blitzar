# sumcheck
Benchmarks sumcheck using elements from the Curve25519 scalar field.

## Running the benchmark
```
bazel run -c opt //benchmark/sumcheck:benchmark -- <n> <degree> <num_products> <num_samples>
```
