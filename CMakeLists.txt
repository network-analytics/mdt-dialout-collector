# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# Project
project(mdt_dialout_collector)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Protobuf gRPC's plugin
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "proto/mdt_dialout_cisco.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Protobuf generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout_cisco.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout_cisco.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout_cisco.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout_cisco.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto")

# Targets
foreach(_target
  "mdt_dialout_collector")
  add_executable(${_target} "${CMAKE_CURRENT_SOURCE_DIR}/src/${_target}.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mdt_dialout_core.cc"
    ${hw_proto_srcs}
    ${hw_grpc_srcs})
  target_link_libraries(${_target}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
