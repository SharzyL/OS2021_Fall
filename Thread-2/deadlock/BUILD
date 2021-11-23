##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_binary(
    name = "main",
    srcs = [
        "main.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "data/example.in",
    ],
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//lib:resource_manager_lib",
        "//lib:thread_manager_lib",
        "//lib:utils_lib",
        "//lib:workload_lib",
        "@com_github_google_glog//:glog",
        "@fmt",
    ],
)