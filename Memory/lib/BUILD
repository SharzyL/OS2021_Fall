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

cc_library(
    name = "memory_manager_lib",
    srcs = [
        "array_list.cc",
        "memory_manager.cc",
    ],
    hdrs = [
        "array_list.h",
        "evict_mgr.h",
        "memory_manager.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@com_github_google_glog//:glog",
        "@fmt",
    ],
)
