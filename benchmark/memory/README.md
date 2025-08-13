# memory
This benchmark evaluates three different strategies for a common GPU memory access pattern: gathering strided data from host memory, performing a reduction operation on the GPU, and copying results back to the host.

Multiple Device Copies (sum1): Performs m separate device copy operations for each data segment, minimizing host-side data rearrangement but potentially increasing GPU transfer overhead.

Host Rearrangement (sum2): Rearranges data on the host into a contiguous buffer before performing a single device copy, trading CPU work for more efficient GPU transfers.

Specialized Strided Copy (sum3): Uses a dedicated strided copy function that optimizes the transfer pattern while maintaining the original data layout, aiming to balance CPU and GPU workloads.

## Running the benchmark
```
bazel run -c opt //benchmark/memory:copy
```
