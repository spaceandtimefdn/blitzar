# reduce2 
Benchmarking for naive scalar multiplication of Curve25519 elements.

Given m and n, we compute res_j for j = 1, ..., m, where

res_j = sum_{i=1,...,n} element_{ij}

where element_{ij} is an element on the elliptical curve 25519.

For each run, we measure the duration in milliseconds and the throughput (m * n / duration).

## Running the benchmark
```
bazel run -c opt //benchmark/reduce2:benchmark <cpu|gpu> <m> <n> <verbose>
```
