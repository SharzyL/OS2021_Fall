cmake_minimum_required(VERSION 3.18)
project(deadlock)

set(CMAKE_CXX_STANDARD 17)

find_package(fmt REQUIRED)
find_package(glog REQUIRED)

find_package(Threads REQUIRED)
add_executable(deadlock main.cc lib/resource_manager.cc lib/thread_manager.cc lib/utils.cc lib/workload.cc)
target_link_libraries(deadlock Threads::Threads fmt glog::glog)
