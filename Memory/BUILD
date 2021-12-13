##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_test(
    name = "mma_test",
    size = "small",
    srcs = ["mma_test.cc"],
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//lib:memory_manager_lib",
        "//lib:utils_lib",
        "@com_github_google_glog//:glog",
        "@fmt",
        "@gtest//:gtest_main",
    ],
)

cc_binary(
    name = "mma_benchmark",
    srcs = ["mma_benchmark.cc"],
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//lib:memory_manager_lib",
        "//lib:utils_lib",
        "@com_github_google_glog//:glog",
        "@fmt",
        "@gbench//:benchmark",
    ],
)
