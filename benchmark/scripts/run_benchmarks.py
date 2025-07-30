#!/usr/bin/env python3

import sys
import subprocess
import argparse


def run_command(cmd):
    """Run a shell command and print output"""
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def main():
    # Set up argument parsing
    parser = argparse.ArgumentParser(description='Run Blitzar benchmarks')
    parser.add_argument('device', choices=['cpu', 'gpu'], help='Device to run benchmarks on')
    args = parser.parse_args()

    # Define benchmark parameters
    sizes = [10000, 100000, 1000000]
    num_samples = 10
    num_commitments = [1, 10]
    element_nbytes = [1, 32]
    verbose = 0

    # Multi commitment benchmark
    for commitment in num_commitments:
        for nbytes in element_nbytes:
            for size in sizes:
                cmd = [
                    "bazel", "run", "-c", "opt", "//benchmark/multi_commitment:benchmark", "--",
                    args.device, str(size), str(num_samples), str(commitment), str(nbytes), str(verbose)
                ]
                run_command(cmd)

    # Inner product proof benchmark
    for size in sizes:
        cmd = [
            "bazel", "run", "-c", "opt", "//benchmark/inner_product_proof:benchmark", "--",
            args.device, str(size), str(num_samples)
        ]
        run_command(cmd)


if __name__ == "__main__":
    main()
