# Benchmarks for the commitment computation

## Description

Given the number of each commitment length, the total number of commitments, and the number of bytes used to represent data elements, we compute each commitment sequence, using for that Curve25519 and Ristretto group operations. The data elements are randomly generated (in a deterministic way, given that the same seed is always provided).

## Available benchmarks
- [inner_product_proof](../benchmark/inner_product_proof/README.md)
- [memory](../benchmark/memory/README.md)
- [multi_commitment](../benchmark/multi_commitment/README.md)
- [multi_exp_pip](../benchmark/multi_exp_pip/README.md)
- [multi_exp_triangle](../benchmark/multi_exp_triangle/README.md)
- [multi_exp1](../benchmark/multi_exp1/README.md)
- [multiprod1](../benchmark/multiprod1/README.md)
- [primitives](../benchmark/primitives/README.md)
- [reduce2](../benchmark/reduce2/README.md)
- [sumcheck](../benchmark/sumcheck/README.md)

## Running the benchmarks
Note all benchmarks are run from inside the NIX development shell. When running benchmarks outside of the shell, add `nix develop --command` before any commands.

To run the whole benchmark suite from outside of the NIX development shell execute:
```
nix develop --command python3 ./benchmark/scripts/run_benchmarks.py <cpu|gpu>
```

From inside the NIX development shell execute:
```
python3 ./benchmark/scripts/run_benchmarks.py <cpu|gpu>
```
