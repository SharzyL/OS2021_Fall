##load("@rules_cc//cc:defs.bzl", "cc_test")  #load the test target

cc_binary(
    name = "q0",
    srcs = [
        "q0.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "//data:q0.in",
        "//data:q0_instruction.tsv",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "format",
    srcs = [
        "format.cc",
    ],
    copts = [
        "-std=c++17",
    ],
)

cc_binary(
    name = "q1",
    srcs = [
        "q1.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "//data:q1.in",
        "//data:q1_instruction.tsv",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "q2",
    srcs = [
        "q2.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "//data:q2.in",
        "//data:q2_instruction.tsv",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "q3",
    srcs = [
        "q3.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "//data:q3.in",
        "//data:q3_instruction.tsv",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
    ],
)

cc_binary(
    name = "q4",
    srcs = [
        "q4.cc",
    ],
    copts = [
        "-std=c++17",
    ],
    data = [
        "//data:q4.in",
        "//data:q4_instruction.tsv",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//lib:embedding_lib",
        "//lib:instruction_lib",
        "//lib:model_lib",
        "//lib:operation_lib",
        "@com_github_google_glog//:glog",
    ],
)

cc_test(
    name = "benchmark",
    size = "small",
    srcs = ["benchmark.cc"],
    copts = [
        "-O3",
        "-std=c++17",
    ],
    data = glob(["data/*"]),
    linkopts = ["-pthread"],
    deps = [
        "@gbench//:benchmark",
    ],
)
