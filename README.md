<div id="top"></div>

<!-- PROJECT LOGO -->
<br />
<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" width="200px" srcset="https://raw.githubusercontent.com/spaceandtimefdn/blitzar-rs/assets/logo_dark_background.png">
  <source media="(prefers-color-scheme: light)" width="200px" srcset="https://raw.githubusercontent.com/spaceandtimefdn/blitzar-rs/assets/logo_light_background.png">
  <img alt="Blitzar" width="200px" src="https://raw.githubusercontent.com/spaceandtimefdn/blitzar-rs/assets/logo_light_background.png">
</picture>

<p align="center">
  <a href="https://github.com/spaceandtimefdn/blitzar/actions/workflows/release.yml">
    <img alt="Build State" src="https://github.com/spaceandtimefdn/blitzar/actions/workflows/release.yml/badge.svg">
  </a>

  <a href="https://twitter.com/intent/follow?screen_name=spaceandtimedb">
    <img alt="Twitter" src="https://img.shields.io/twitter/follow/spaceandtimedb.svg?style=social&label=Follow">
  </a>

  <a href="http://discord.gg/SpaceandTimeDB">
    <img alt="Discord Server" src="https://img.shields.io/discord/953025874154893342?logo=discord">
  </a>

  <a href="https://github.com/spaceandtimefdn/blitzar/blob/main/LICENSE">
    <img alt="License" src="https://img.shields.io/badge/License-Apache_2.0-blue.svg">
    </a>

  <a href="https://en.cppreference.com/w/cpp/20">
    <img alt="C++ Logo" src="https://img.shields.io/badge/C%2B%2B-20-blue?style=flat&logo=c%2B%2B">
    </a>

  <a href="https://www.linux.org/">
    <img alt="OS" src="https://img.shields.io/badge/OS-Linux-blue?logo=linux">
    </a>

  <a href="https://www.linux.org/">
    <img alt="CPU" src="https://img.shields.io/badge/CPU-x86-red">
    </a>

  <a href="https://developer.nvidia.com/cuda-downloads">
    <img alt="CUDA" src="https://img.shields.io/badge/CUDA-12.6-green?style=flat&logo=nvidia">
    </a>

  <p align="center">
    Space and Time C++ library for accelerating cryptographic zero-knowledge proofs algorithms on the CPU and GPU.
    <br />
    <a href="https://github.com/spaceandtimefdn/blitzar/issues">Report Bug</a>
    |
    <a href="https://github.com/spaceandtimefdn/blitzar/issues">Request a Feature</a>
  </p>
</div>

#### Background
Blitzar was created by the core cryptography team at [Space and Time](https://www.spaceandtime.io/) to accelerate Proof of SQL, a novel zero-knowledge proof for SQL operations. After surveying our options for a GPU acceleration framework, we realized that Proof of SQL needed something better… so we built Blitzar. Now, Proof of SQL can execute analytic queries on 1M+ rows in less than a second, and it’s only getting faster.

We’ve open-sourced Blitzar to provide the Web3 community with a faster and more
robust framework for building GPU-accelerated ZK proofs. We’re excited to open
the project to community contributions to expand the scope of Blitzar and lay
the foundation for the next wave of lightning fast ZK proofs.

#### Overview
Blitzar is a C++ library for accelerating cryptographic zero-knowledge proof algorithms on the CPU and GPU.
> **Note**  
> This repo contains the C++ implementation along with cbindings and a Rust sys-crate. If you are using Rust, use the crate from the companion [blitzar-rs](https://github.com/spaceandtimefdn/blitzar-rs) repo.

The library provides

* Functions for doing group operations on [Curve-25519](https://en.wikipedia.org/wiki/Curve25519), [Ristretto25519](https://ristretto.group/), [bls12-381 G1](https://electriccoin.co/blog/new-snark-curve/), [bn254 G1](https://hackmd.io/@jpw/bn254) and [Grumpkin](https://hackmd.io/@aztec-network/B19AA8812#Curve-cycles) elements.
* An implementation of [Inner Product Argument Protocol](https://eprint.iacr.org/2017/1066.pdf) for producing and verifying a compact proof of the inner product of two vectors.
* A sys-crate and bindings to make commitment computations usable from [Rust](https://github.com/spaceandtimefdn/blitzar-rs).

The library is adopted from code in the [libsodium](https://github.com/jedisct1/libsodium) and [zkcrypto](https://github.com/zkcrypto/bls12_381) projects. It extends both project's cryptographic functions to support CUDA so that they are usable on GPUs.

#### Installation
We provide prebuilt binaries for glibc-based, x86-64 linux distributions.

Dependencies are statically linked and set to have internal linkage with [export maps](https://accu.org/journals/overload/15/79/wakeling_1372/) to ensure 
portability. The only run-time dependency to use GPU acceleration is an up-to-date GPU driver.

For most users, we recommend installing with cargo via [blitzar-rs](https://github.com/spaceandtimefdn/blitzar-rs).

Alternatively, users that want to use the c api directly can download the shared library and header file from the github release.

#### Computational Backends
Although the primary goal of this library is to provide GPU acceleration for cryptographic ZK proof algorithms, the library also provides CPU support for the sake of testing. The following backends are supported:

| Backend            | Implementation                                             | Target Hardware             |
| :---               | :---                                                       | :---                        |
| `cpu`              | Serial                                                     | x86 capable CPUs            |
| `gpu`              | Parallel                                                   | Nvidia CUDA capable GPUs    |


## Cryptographic Primitives

#### Multi-Scalar Multiplication (MSM) / Generalized Pedersen Commitment / Multiexponentiation

Blitzar provides an implementation of Multi-Scalar Multiplication (i.e. generalized Pedersen commitments).

Let $g_0\ldots g_n\in \mathbb{G}$ be elements of a group (with prime order), and let $a_0\ldots a_n\in\mathbb{F}$ be elements of the corresponding scalar field. (i.e. the field $\mathbb{F}_p$ where $p$ is the order of the group.)

Then, the Generalized Pedersen Commitment of the vector $\mathbf{a}=(a_1,\ldots, a_n)$ is
```math
P = a_1\cdot g_1+\cdots+ a_n\cdot g_n
```

Note: we interchangeably use the terms "multi-scalar multiplication" and "multiexponentiation" to refer to the this operation because when the group is written additively, the operation is a multi-scalar multiplication, and when the group is written multiplicatively, the operation is a multiexponentiation.

The Blitzar implementation allows for computation of multiple, potentially different length, MSMs simultaneously. Additionally, either built-in, precomputed, generators $g_n$ can be used, or they can be provided as needed.

Currently, Blitzar supports group elements from the Curve25519, bls12-381 G1, bn254-381 G1, and Grumpkin curves.

#### Inner Product Argument

Blitzar provides a modified implementation of an inner product argument (e.g. [Bulletproofs](https://eprint.iacr.org/2017/1066.pdf) and [Halo2](https://zcash.github.io/halo2/background/pc-ipa.html)).

Given generators $g_1, \ldots, g_n$; Pedersen commitment $P$; scalar $c$; and vectors 
$\mathbf{a}=(a_1,\ldots, a_n)$ and $\mathbf{b}=(b_1,\ldots, b_n)$;
Blitzar's version of the inner product proof allows a Prover to establish that
```math
\begin{aligned}
P &= a_1\cdot g_1+\cdots+ a_n\cdot g_n \\
c &= \langle \mathbf{a}, \mathbf{b} \rangle = a_1\cdot b_1+\cdots+ a_n\cdot b_n
\end{aligned}
```
where it is assumed that $\boldsymbol{g}$, $\boldsymbol{b}$, and $c$ are known to both the Prover
and Verifier.

This version of the inner product argument can be used in the context of a broader protocol.

#### Other Features to Come

If there is a particular feature that you would like to see, please [reach out](https://github.com/spaceandtimefdn/blitzar/issues). Blitzar is a community-first project, and we want to hear from you.

## Performance ([associated commit hash](https://github.com/spaceandtimefdn/blitzar/commit/7be4bdfc961e7f0b99b99bf22238380829a325d2))

Benchmarks are run against four different types of GPU:

* [Nvidia T4 - Standard_NC4as_T4_v3](https://learn.microsoft.com/en-us/azure/virtual-machines/nct4-v3-series)
* [Nvidia T4 x4 - Standard_NC16as_T4_v3](https://learn.microsoft.com/en-us/azure/virtual-machines/nct4-v3-series)
* [Nvidia A100 - Standard_NC24ads_A100_v4](https://learn.microsoft.com/en-us/azure/virtual-machines/nc-a100-v4-series)
* [Nvidia A100 x4 - Standard_NC96ads_A100_v4](https://learn.microsoft.com/en-us/azure/virtual-machines/nc-a100-v4-series)

<details>
<summary>Expand to see the multi-scalar multiplication / generalized Pedersen commitment results</summary>

The subsequent outcomes are derived from the preceding benchmark execution of the Pedersen commitment, during which the number of sequences, bytes per element, sequence length, and GPU type were varied.

![BN254 Multiexponentiation Benchmarks](https://github.com/spaceandtimefdn/blitzar/blob/assets/benchmark/bn254_blitzar_compute_commitments.png)
![BLS12-381 Multiexponentiation Benchmarks](https://github.com/spaceandtimefdn/blitzar/blob/assets/benchmark/bls12_381_blitzar_compute_commitments.png)
![Grumpkin Multiexponentiation Benchmarks](https://github.com/spaceandtimefdn/blitzar/blob/assets/benchmark/grumpkin_blitzar_compute_commitments.png)
![Curve25519 Multiexponentiation Benchmarks](https://github.com/spaceandtimefdn/blitzar/blob/assets/benchmark/curve25519_blitzar_compute_commitments.png)

</details>

## Getting Started

See the [example](./example) folder for some examples.

### Prerequisites to build from source

<details open>
<summary>Build environment</summary>

Prerequisites:
* `x86_64` Linux instance.
* Nix with flake support (check out [The Determinate Nix Installer](https://github.com/DeterminateSystems/nix-installer)).
* Nvidia GPU capable of running CUDA 12.6 code.

From your terminal, run the following command in the root of the source directory to set
up a build environment. 
```bash
nix develop
```
Note: if this is the first time, it may take a while as we build a clang compiler from source.
</details>

## Usage

<details open>
<summary>Building and Testing the C++/CUDA code:</summary>

```bash
nix develop

# build all the code assets
bazel build //...

# run all tests
bazel test //...

# run all memory sanitized tests
bazel test --config=asan //...
```

Note: some tests will fail in case you don't have a GPU available.

</details>

<details open>
<summary>Building and Testing the Rust Sys-Crate code:</summary>

```bash
nix develop

# run the sys-crate tests
cargo test --manifest-path rust/blitzar-sys/Cargo.toml
```

Although possible, this sys-crate is not meant to be used directly by Rust users. Instead, consider using the [blitzar-rs](https://github.com/spaceandtimefdn/blitzar-rs), which is a high-level wrapper around this sys-crate.

Note: the shared library byproduct of the C++/CUDA code is automatically copied to the Rust sys-crate under the `rust/blitzar-sys/` directory.

</details>

## Add to your project

You can find release ready versions of this library under the [release page](https://github.com/spaceandtimefdn/blitzar/releases/). You can also build and test the library from source by following the instructions below.

<details>

<summary>C++ Project:</summary>

See the C++ example here: [example/cbindings1/main.cc](./example/cbindings1/main.cc). To run this example, execute:

```bash
nix develop

bazel run //example/cbindings1:cbind1
```

Alternatively, compile this example code with g++:

```bash
nix develop

# Build the shared library
bazel build -c opt --config=portable_glibc //cbindings:libblitzar.so

# Compile the C++ example code
g++ example/cbindings1/main.cc -o main -I . -L bazel-bin/cbindings/ -lblitzar

# Execute the example code
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:bazel-bin/cbindings/ ./main
```
</details>

<details>

<summary>Rust Project:</summary>

See the Rust example here: [rust/tests/src/main.rs](./rust/tests/src/main.rs). To run this example, execute:

```bash
nix develop

cargo test --manifest-path rust/tests/Cargo.toml
```

</details>

## Development Process

The main branch is regularly built and tested, being the only source of truth. [Tags](https://github.com/spaceandtimefdn/blitzar/tags) are created regularly from automated semantic release executions. 

#### Code format:

This repository follows some C++ formatting rules. To check if your modified code is compliant, run the following commands:

```bash
nix develop

./tools/code_format/check_format.py check

./tools/code_format/check_format.py fix
```

## Executing the Benchmarks:

<details open>
<summary>Generalized Pedersen Commitment (MSM):</summary>

```bash
nix develop

# Usage: benchmark <cpu|gpu> <curve> <n> <num_samples> <num_commitments> <element_nbytes> <verbose>
# - curve: the desired curve: curve25519, bn254, bls12_381, grumpkin
# - n: the size of the multiexponentiation vector (or the sequence length).
# - num_samples: the number of times to run the benchmark.
# - num_commitments: the number of commitments to generate.
# - element_nbytes: the number of bytes of each element in the vector (exponent size).
# - verbose: whether to print the results of each run.
bazel run -c opt //benchmark/multi_commitment:benchmark -- gpu bn254 1000 5 1 32 1
```

</details>

<details open>
<summary>Inner Product Argument:</summary>

```bash
nix develop

# Usage: benchmark <cpu|gpu> <n> <num_samples>
# - n: the size of the inner product vector (or the sequence length).
# - num_samples: the number of times to run the benchmark.
bazel run -c opt //benchmark/inner_product_proof:benchmark -- gpu 1000 5
```

</details>


## Contributing

We're excited to open Blitzar to the community, but are not accepting community Pull Requests yet due to logistic reasons. However, feel free to contribute with any suggestion, idea, or bugfix on our [Issues](https://github.com/spaceandtimefdn/blitzar/issues) panel. Also, see [contribution guide](CONTRIBUTING.md).

## Community & support

Join our [Discord server](https://discord.com/invite/SpaceandTimeDB) to ask questions, discuss features, and for general chat.

## License

This project is released under the [Apache 2 License](LICENSE).
  
## Rust crate

This repo contains the C++ implementation along with cbindings and a Rust sys-crate. If you are using Rust, use the crate from the companion repo here: https://github.com/spaceandtimefdn/blitzar-rs.
