cmake_minimum_required(VERSION 3.22)
project(Memory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

include_directories(lib)

add_subdirectory(demo)

find_package(Threads)

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
FetchContent_MakeAvailable(googletest)

include(FetchContent)
FetchContent_Declare(gRPC GIT_REPOSITORY https://github.com/grpc/grpc GIT_TAG v1.43.0)
FetchContent_MakeAvailable(gRPC)

enable_testing()

add_executable(mma_test mma_test.cc lib/mma_client.cc lib/mma_server.cc lib/memory_manager.cc lib/array_list.cc)

find_package(fmt)
find_package(glog)
find_package(benchmark)
include_directories(bazel-bin)
include_directories(bazel-bin/proto)

target_link_libraries(mma_test Threads::Threads gtest_main fmt glog::glog grpc++)

include(GoogleTest)
gtest_discover_tests(mma_test)
