# Benchmarks for the commitment computation

## Description

Given the number of each commitment length, the total number of commitments, and the number of bytes used to represent data elements, we compute each commitment sequence, using for that Curve25519 and Ristretto group operations. The data elements are randomly generated (in a deterministic way, given that the same seed is always provided).

## Running the benchmarks

To run the whole benchmark suite from outside of the NIX development shell execute:

```
nix develop --command python3 ./benchmark/scripts/run_benchmarks.py <cpu|gpu>
```

From inside the NIX development shell execute:
```
python3 ./benchmark/scripts/run_benchmarks.py <cpu|gpu>
```
