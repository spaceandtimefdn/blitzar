# multiprod1
Benchmarks for the multiproduct computation that compare both the naive and the [Pippenger multiproduct](https://cacr.uwaterloo.ca/techreports/2010/cacr2010-26.pdf) implementation for Curve25519 elements.

## Running the benchmark
```
bazel run -c opt //benchmark/multiprod1:benchmark <use_naive> <sequence_length> <num_sequences> <max_num_inputs> <num_samples> <verbose> 
```
