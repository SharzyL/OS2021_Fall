cmake_minimum_required(VERSION 3.22)
project(Memory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

find_package(Threads)
include(FindProtobuf)
find_package(Protobuf REQUIRED)

include(FetchContent)
FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.43.0  # e.g v1.28.0
)
FetchContent_MakeAvailable(gRPC)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER proto/helloworld.proto)
add_library(proto_helloworld ${PROTO_SRC} ${PROTO_HEADER})

set(Targets greeter_client greeter_server.cc)
foreach(t IN LISTS Targets)
    add_executable(${t} greeter_client.cc)
    message(${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(${t} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${t} grpc++ protobuf::libprotobuf)
endforeach()
