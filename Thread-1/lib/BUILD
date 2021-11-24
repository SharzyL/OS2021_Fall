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
    name = "embedding_lib",
    srcs = [
        "embedding.cc",
    ],
    hdrs = [
        "embedding.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":utils_lib",
    ],
)

cc_test(
    name = "embedding_lib_test",
    size = "small",
    srcs = ["embedding_test.cc"],
    deps = [
        ":embedding_lib",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "instruction_lib",
    srcs = [
        "instruction.cc",
    ],
    hdrs = [
        "instruction.h",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "model_lib",
    srcs = [
        "model.cc",
    ],
    hdrs = [
        "model.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":embedding_lib",
        ":utils_lib",
    ],
)

cc_test(
    name = "model_lib_test",
    size = "large",
    srcs = ["model_test.cc"],
    deps = [
        ":model_lib",
        "@gtest//:gtest_main",
    ],
)

cc_library(
    name = "operation_lib",
    srcs = [
        "operation.cc",
    ],
    hdrs = [
        "operation.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":embedding_lib",
        ":instruction_lib",
        ":model_lib",
        ":utils_lib",
        "@com_github_google_glog//:glog",
        "@fmt",
    ],
)

cc_test(
    name = "operation_lib_test",
    size = "large",
    srcs = ["operation_test.cc"],
    data = ["//data:q1.in"],
    deps = [
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
        "@gtest//:gtest_main",
    ],
)
