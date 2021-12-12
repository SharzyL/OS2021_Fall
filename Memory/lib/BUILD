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
        "memory_manager.cc",
        "array_list.cc",
        ],
    hdrs = [
        "memory_manager.h",
        "array_list.h",
        ],
    deps = [
        "@com_github_google_glog//:glog",
        "@fmt",
    ],
	visibility = [
		"//visibility:public",
	],
)


