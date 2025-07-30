# multi_exp1
Benchmarking for naive scalar multiplication using Curve25519 group elements.

Given m and n, we compute res_j for j = 1, ..., m, where

res_j = sum_{i=1,...,n} a_{ij} * element_{ij}

where element_{ij} is an element on the elliptical curve 25519 and
a_{ij} is a random 256-bit value.

For each run, we measure the duration in milliseconds and the throughput (m * n / duration).

## Running the benchmark
```
bazel run -c opt //benchmark/multi_exp1:benchmark -- <cpu|gpu> <m> <n> <verbose>
```
