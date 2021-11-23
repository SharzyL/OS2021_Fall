##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_library(
    name = "utils_lib",
    srcs = [
        "utils.cc",
    ],
    hdrs = [
        "utils.h",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_test(
    name = "utils_lib_test",
    size = "small",
    srcs = ["util_test.cc"],
    deps = [
        ":utils_lib",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "resource_manager_lib",
    srcs = [
        "resource_manager.cc",
    ],
    hdrs = [
        "resource_manager.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":thread_manager_lib",
        "@com_github_google_glog//:glog",
        "@fmt",
    ],
)

cc_test(
    name = "resource_manager_lib_test",
    size = "small",
    srcs = ["resource_manager_test.cc"],
    deps = [
        ":resource_manager_lib",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "workload_lib",
    srcs = [
        "workload.cc",
    ],
    hdrs = [
        "workload.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":resource_manager_lib",
        ":utils_lib",
    ],
)

cc_test(
    name = "workload_lib_test",
    size = "small",
    srcs = ["workload_test.cc"],
    deps = [
        ":workload_lib",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "thread_manager_lib",
    srcs = [
        "thread_manager.cc",
    ],
    hdrs = [
        "thread_manager.h",
    ],
    visibility = [
        "//visibility:public",
    ],
)