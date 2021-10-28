cmake_minimum_required(VERSION 3.21)
project(Thread_1)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.6.0) # need master for benchmark::benchmark

FetchContent_MakeAvailable(
        googletest
        googlebenchmark)

find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(glog REQUIRED)

include_directories(.)
set(Lib lib/embedding.cc lib/instruction.cc lib/utils.cc lib/model.cc lib/operation.cc)
add_library(lib ${Lib})

set(Targets q0 q1 q2 q3 q4)
foreach(target IN LISTS Targets)
    add_executable(${target} ${target}.cc)
    target_link_libraries(${target} lib fmt Threads::Threads glog::glog)
endforeach()

set(TestTargets embedding model util)
foreach(target IN LISTS TestTargets)
    add_executable(${target}_test lib/${target}_test.cc)
    target_link_libraries(${target}_test lib gtest_main)
endforeach()

add_executable(operation_test lib/operation_test.cc)
target_link_libraries(operation_test lib fmt Threads::Threads gtest_main glog::glog)
