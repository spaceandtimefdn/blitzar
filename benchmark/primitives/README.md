# Benchmarking for primitives on the GPU
Currently only supports benchmarks for addition with `bls12-381` curve elements. Creates two vectors of size `<n_elements>` and does vector addition with them `<repetitions>` number of times. Timing results are output in milliseconds.

This benchmark is similar to the Icicle's [example/multiply](https://github.com/ingonyama-zk/icicle/tree/40309329fbf6c5fc7e77d629c72b4a3d28036444/examples/c%2B%2B/multiply), but updated to measure curve addition, field addition, and field multiplication.

## Running the benchmark
```
bazel run -c opt //benchmark/primitives:benchmark <curve> <op> <n_elements> <repetitions> <optional - n_threads> <optional - n_executions>
```

- `curve`: specifies the curve. Currently only supports `bls12_381`
- `op`: specifies what operations to target, either `curve` or `field`
- `n_elements`: specifies the number of elements in the vector
- `repetitions`: specifies the number of operations executed for each element of the vector
- `n_threads`: specifies the max number of threads per block
- `n_executions`: specifies the number of executions used when measuring benchmark statistics

## Example
Run curve operations using `bls12-381` elements.  
`bazel run -c opt //benchmark/primitives:benchmark bls12_381 curve 10000 1000 256 10`

Run curve operations using `bn254` elements.  
`bazel run -c opt //benchmark/primitives:benchmark bn254 curve 10000 1000 256 10`

Run field operations (addition and multiplication) using `bls12-381` elements.  
`bazel run -c opt //benchmark/primitives:benchmark bls12_381 field 1000000 1000 256 10`

Run field operations (addition and multiplication) using `bn254` elements.  
`bazel run -c opt //benchmark/primitives:benchmark bn254 field 1000000 1000 256 10`
