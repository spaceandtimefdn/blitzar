load("//bazel:sxt_benchmark.bzl", "sxt_cc_benchmark")

sxt_cc_benchmark(
    name = "benchmark",
    srcs = [
        "benchmark.m.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//sxt/base/container:span",
        "//sxt/base/error:panic",
        "//sxt/base/num:ceil_log2",
        "//sxt/base/num:fast_random_number_generator",
        "//sxt/execution/async:future",
        "//sxt/execution/schedule:scheduler",
        "//sxt/fieldgk/random:element",
        "//sxt/fieldgk/realization:field",
        "//sxt/memory/management:managed_array",
        "//sxt/proof/sumcheck:chunked_gpu_driver",
        "//sxt/proof/sumcheck:cpu_driver",
        "//sxt/proof/sumcheck:gpu_driver",
        "//sxt/proof/sumcheck:proof_computation",
        "//sxt/proof/sumcheck:reference_transcript",
        "//sxt/proof/sumcheck:verification",
        "//sxt/proof/transcript",
        "//sxt/scalar25/random:element",
        "//sxt/scalar25/realization:field",
    ],
)
