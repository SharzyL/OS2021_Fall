cmake_minimum_required(VERSION 3.22)
project(Memory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

include_directories(lib)

find_package(Threads)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(mma_test
        lib/array_list.cc
        lib/memory_manager.cc
        lib/utils.cc
        mma_test.cc)

add_executable(mma_benchmark_extra
        lib/array_list.cc
        lib/memory_manager.cc
        lib/utils.cc
        mma_benchmark_extra.cc)

add_executable(mma_benchmark
        lib/array_list.cc
        lib/memory_manager.cc
        lib/utils.cc
        mma_benchmark.cc)

find_package(fmt)
find_package(glog)
find_package(benchmark)

target_link_libraries(mma_test Threads::Threads gtest_main fmt glog::glog)
target_link_libraries(mma_benchmark Threads::Threads benchmark fmt glog::glog)
target_link_libraries(mma_benchmark_extra Threads::Threads benchmark fmt glog::glog)

include(GoogleTest)
gtest_discover_tests(mma_test)
