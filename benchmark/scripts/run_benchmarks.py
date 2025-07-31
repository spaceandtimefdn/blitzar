#!/usr/bin/env python3

import sys
import subprocess
import argparse


def run_command(cmd):
    """Run a shell command and print output"""
    print(f"Running: {' '.join(cmd)}")
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: Command '{' '.join(cmd)}' failed with return code {e.returncode}")
        print(f"Output: {e.output.decode('utf-8') if e.output else 'No output'}")
        sys.exit(1)

def main():
    # Set up argument parsing
    parser = argparse.ArgumentParser(description='Run Blitzar benchmarks')
    parser.add_argument('device', choices=['cpu', 'gpu'], help='Device to run benchmarks on')
    args = parser.parse_args()

    # Define benchmark parameters
    curves = ["curve25519", "bls12_381", "bn254", "grumpkin"]
    sizes = [10000, 100000, 1000000]
    num_samples = 10
    num_commitments = [1, 10]
    element_nbytes = [1, 32]
    verbose = 0

    # Multi commitment benchmark
    print("Running multi commitment benchmarks...")
    for curve in curves:
        for commitment in num_commitments:
            for nbytes in element_nbytes:
                for size in sizes:
                    cmd = [
                        "bazel", "run", "-c", "opt", f"//benchmark/multi_commitment:benchmark", "--",
                        args.device, str(curve), str(size), str(num_samples), str(commitment), str(nbytes), str(verbose)
                    ]
                    run_command(cmd)

    # Multi exponentiation Pippenger benchmark
    print("Running multi exponentiation Pippenger benchmarks...")
    for curve in curves:
        for commitment in num_commitments:
            for nbytes in element_nbytes:
                for size in sizes:
                    cmd = [
                        "bazel", "run", "-c", "opt", f"//benchmark/multi_exp_pip:benchmark", "--",
                        str(curve), str(size), str(num_samples), str(commitment), str(nbytes), str(verbose)
                    ]
                    run_command(cmd)

    # Multi exponentiation triangle benchmark
    print("Running multi exponentiation triangle benchmarks...")
    for curve in curves:
        for commitment in num_commitments:
            for nbytes in element_nbytes:
                for size in sizes:
                    cmd = [
                        "bazel", "run", "-c", "opt", f"//benchmark/multi_exp_tri:benchmark", "--",
                        str(curve), str(size), str(num_samples), str(commitment), str(nbytes), str(verbose)
                    ]
                    run_command(cmd)

    # Inner product proof benchmark
    print("Running inner product proof benchmarks...")
    for size in sizes:
        cmd = [
            "bazel", "run", "-c", "opt", "//benchmark/inner_product_proof:benchmark", "--",
            args.device, str(size), str(num_samples)
        ]
        run_command(cmd)

    # Sumcheck benchmark
    sumcheck_fields = ["curve25519", "bn254"]
    sumcheck_n = [1048576, 2097152, 4194304] # [2**20, 2**21, 2**22]
    sumcheck_degree = [3]
    sumcheck_num_products = [2]
    sumcheck_num_samples = [5]

    print("Running sumcheck benchmarks...")
    for field in sumcheck_fields:
        for n in sumcheck_n:
            for degree in sumcheck_degree:
                for num_products in sumcheck_num_products:
                    for num_samples in sumcheck_num_samples:
                        cmd = [
                            "bazel", "run", "-c", "opt", "//benchmark/sumcheck:benchmark", "--",
                            str(field), str(n), str(degree), str(num_products), str(num_samples)
                        ]
                        run_command(cmd)


if __name__ == "__main__":
    main()
