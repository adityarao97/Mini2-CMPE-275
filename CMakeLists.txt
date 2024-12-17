cmake_minimum_required(VERSION 3.30)
project(mini2)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-std=c++17)
add_compile_options(-g)

### Package START

find_package(Protobuf CONFIG)
message(STATUS "Protobuf version ${Protobuf_VERSION}")

find_package(gRPC CONFIG)
message(STATUS "gRPC version ${gRPC_VERSION}")

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/crow/1.2.0/lib/cmake/Crow/")

find_package(crow CONFIG)
message(STATUS "Crow version ${crow_VERSION}")

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/libomp/19.1.4/")
find_package(OpenMP)
message(STATUS "OpenMP version ${OpenMP_VERSION}")

### Package END
include_directories("/opt/homebrew/Cellar/crow/1.2.0/include/crow.h")
include_directories("generated-src")
include_directories("src/csv_processor.h")
include_directories("/opt/homebrew/Cellar/libomp/19.1.4/include/")

add_executable(
    ping-server 
    src/ping_server.cc
    generated-src/reader.grpc.pb.cc # add the .cc file
    generated-src/reader.pb.cc
)

add_executable(
    grpc-server
    src/grpc_server.cc
    generated-src/reader.grpc.pb.cc
    generated-src/reader.pb.cc
)

add_executable(
    grpc-client
    src/grpc_client.cc
    generated-src/reader.grpc.pb.cc
    generated-src/reader.pb.cc
)

add_executable(
    http-server
    src/http_server.cc
)

add_executable(
    ping-client
    src/ping_client.cc
    generated-src/reader.grpc.pb.cc
    generated-src/reader.pb.cc
)

target_link_libraries(
    ping-client
    gRPC::grpc++_reflection
    protobuf::libprotobuf 
)

target_link_libraries(
    ping-server
    gRPC::grpc++_reflection
    protobuf::libprotobuf 
)

target_link_libraries(
    grpc-server
    gRPC::grpc++_reflection
    protobuf::libprotobuf 
    absl::flags absl::flags_parse OpenMP::OpenMP_CXX
)

target_link_libraries(
    grpc-client
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_link_libraries(
    http-server
    Crow::Crow
    absl::flags absl::flags_parse OpenMP::OpenMP_CXX
)