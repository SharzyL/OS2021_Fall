cmake_minimum_required(VERSION 3.22)
project(Memory)

set(CMAKE_CXX_STANDARD 14)

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
        lib/array_list.h
        lib/memory_manager.cc
        lib/memory_manager.h
        lib/utils.cc
        lib/utils.h
        mma_test.cc)

target_link_libraries(mma_test Threads::Threads gtest_main)

include(GoogleTest)
gtest_discover_tests(mma_test)