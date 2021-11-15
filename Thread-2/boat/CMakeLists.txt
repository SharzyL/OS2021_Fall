cmake_minimum_required(VERSION 3.18)
project(boat)

set(CMAKE_CXX_STANDARD 17)

find_package(fmt REQUIRED)
find_package(glog REQUIRED)

find_package(Threads REQUIRED)
add_executable(boat main.cc boat.cc boatGrader.cc)
target_link_libraries(boat Threads::Threads fmt glog::glog)
