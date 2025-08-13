load("//bazel:sxt_benchmark.bzl", "sxt_cc_benchmark")

sxt_cc_benchmark(
    name = "benchmark",
    srcs = [
        "benchmark.m.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//sxt/base/container:span",
        "//sxt/base/curve:element",
        "//sxt/base/profile:callgrind",
        "//sxt/cbindings/backend:cpu_backend",
        "//sxt/cbindings/backend:gpu_backend",
        "//sxt/curve21/operation:add",
        "//sxt/curve21/operation:double",
        "//sxt/curve21/operation:neg",
        "//sxt/curve21/type:element_p3",
        "//sxt/memory/management:managed_array",
        "//sxt/multiexp/base:exponent_sequence",
        "//sxt/ristretto/base:byte_conversion",
        "//sxt/ristretto/type:compressed_element",
        "//sxt/seqcommit/generator:base_element",
    ],
)
