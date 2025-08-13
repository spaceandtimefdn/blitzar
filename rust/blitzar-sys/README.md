<div id="top"></div>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h1 align="center">Blitzar-sys Crate</h1>

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
  </a>

  <a href="https://en.cppreference.com/w/cpp/20">
    <img alt="C++ Logo" src="https://img.shields.io/badge/C%2B%2B-20-blue?style=flat&logo=c%2B%2B">
    </a>
  </a>

  <a href="https://www.linux.org/">
    <img alt="OS" src="https://img.shields.io/badge/OS-Linux-blue?logo=linux">
    </a>
  </a>

  <a href="https://www.linux.org/">
    <img alt="CPU" src="https://img.shields.io/badge/CPU-x86-red">
    </a>
  </a>

  <a href="https://developer.nvidia.com/cuda-downloads">
    <img alt="CUDA" src="https://img.shields.io/badge/CUDA-12.1-green?style=flat&logo=nvidia">
    </a>
  </a>

  <p align="center">
    Rust bindings for the Blitzar C++ library.
    <br />
    <a href="https://github.com/spaceandtimefdn/blitzar/issues">Report Bug</a>
    |
    <a href="https://github.com/spaceandtimefdn/blitzar/issues">Request a Feature</a>
  </p>
</div>

#### Background

Blitzar was created by the core cryptography team at [Space and Time](https://www.spaceandtime.io/) to accelerate Proof of SQL, a novel zero-knowledge proof for SQL operations.

#### Overview

The `blitzar-sys` crate provides Rust bindings for the [Blitzar C++ Library](https://github.com/spaceandtimefdn/blitzar). The crate is used by Space and Time's companion crate, [`blitzar`](https://crates.io/crates/blitzar), that provides a High-Level Rust wrapper for accelerating cryptographic zero-knowledge proof algorithms on the CPU and GPU.

More information about the cryptographic primitives is available on the [Blitzar Github repo](https://github.com/spaceandtimefdn/blitzar). More information about the Rust bindings are available on the [`blitzar`](https://crates.io/crates/blitzar) crates.io page.

**WARNING**: This project has not undergone a security audit and is NOT ready for production use.

## Community & support

Join our [Discord server](https://discord.com/SpaceandTimeDB) to ask questions, discuss features, and for general chat.

## License

This project is released under the [Apache 2 License](LICENSE).
