# multi_exp_pip
Benchmarking for a multi-exponentiation using an accessor to precomputed sums of partition groups. Runs benchmarks on the following curve elements:
- `curve25519`
- `bls12-381 G1`
- `bn254 G1`
- `grumpkin`

## Running the benchmark
```sh
bazel run -c opt //benchmark/multi_exp_pip:benchmark <curve> <n> <num_samples> <num_outputs> <element_num_bytes> <verbose>
```
- `curve` - the curve to benchmark. Current support: `curve25519`, `bls12-381`, `bn254`, `grumpkin`
- `n` - the number of generators
- `num_samples` - how many times to run the benchmark
- `num_outputs` - the number of commitments in a batched MSM
- `element_num_bytes` - the number of bytes per element
- `verbose` - print verbose output, either `0` or `1`

### Example
```sh
bazel run -c opt //benchmark/multi_exp_pip:benchmark curve25519 1024 30 1024 32 0
```
