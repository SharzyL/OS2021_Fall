package(default_visibility = ["//visibility:public"])

cc_library(
    name = "mma_client_lib",
    srcs = [
        "mma_client.cc",
        "array_list.cc",
    ],
    hdrs = [
        "mma_client.h",
        "evict_mgr.h",
        "const.h",
        "array_list.h",
        ],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//proto:mma_grpc",
        "memory_manager_lib",
    ],
)

cc_library(
    name = "mma_server_lib",
    srcs = [
        "mma_server.cc",
        "memory_manager.cc",
    ],
    hdrs = [
        "mma_server.h",
        "memory_manager.h",
        "evict_mgr.h",
        "const.h",
        ],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//proto:mma_grpc",
        "memory_manager_lib",
    ],
)

cc_library(
    name = "utils_lib",
    srcs = [
        "utils.cc",
        ],
    hdrs = [
        "utils.h",
        ],
)

cc_library(
    name = "memory_manager_lib",
    srcs = [
        "memory_manager.cc",
    ],
    hdrs = [
        "const.h",
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
