cc_library(
    name = "libbacktrace",
    linkstatic = 1,
    srcs = [
        "atomic.c",
        "backtrace.c",
        "backtrace-supported.h",
        "config.h",
        "dwarf.c",
        "elf.c",
        "fileline.c",
        "filenames.h",
        "internal.h",
        "mmap.c",
        "mmapio.c",
        "posix.c",
        "print.c",
        "simple.c",
        "sort.c",
        "state.c",
    ],
    hdrs = [
        "backtrace.h",
    ],
    includes = [
        ".",
    ],
    visibility = [
        "//visibility:public",
    ],
)

genrule(
    name = "config",
    srcs = ["@dev_spaceandtime_blitzar//bazel/libbacktrace:config.h"],
    outs = ["config.h"],
    cmd = "cat $(location @dev_spaceandtime_blitzar//bazel/libbacktrace:config.h) > $@",
)

genrule(
    name = "backtrace-supported",
    srcs = ["@dev_spaceandtime_blitzar//bazel/libbacktrace:backtrace-supported.h"],
    outs = ["backtrace-supported.h"],
    cmd = "cat $(location @dev_spaceandtime_blitzar//bazel/libbacktrace:backtrace-supported.h) > $@",
)
