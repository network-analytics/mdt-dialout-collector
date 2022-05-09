# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# Project
project(mdt_dialout_collector)

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Protobuf gRPC's plugin
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file gRPC dial-out
get_filename_component(hw_proto "proto/mdt_dialout.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Proto file with the definitions required to introduce GPB-KV encoding
get_filename_component(telemetry_proto "proto/telemetry.proto" ABSOLUTE)
get_filename_component(telemetry_proto_path "${telemetry_proto}" PATH)

find_package(PkgConfig REQUIRED)

# jsoncpp libs
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
message(STATUS "Using jsoncpp ${JSONCPP_VERSION}")
set(JSONCPP_INCLUDE_DIR /usr/include/jsoncpp)
include_directories("${JSONCPP_INCLUDE_DIR}")

# librdkafka
pkg_check_modules(RDKAFKA_CPP rdkafka++)
link_libraries(${RDKAFKA_CPP_LIBRARIES})
message(STATUS "Using librdkafka++ ${RDKAFKA_CPP_VERSION}")
set(RDKAFKA_CPP_INCLUDE_DIR /usr/local/include)
include_directories("${RDKAFKA_CPP_INCLUDE_DIR}")

pkg_check_modules(RDKAFKA rdkafka)
link_libraries(${RDKAFKA_LIBRARIES})
message(STATUS "Using librdkafka ${RDKAFKA_VERSION}")
set(RDKAFKA_INCLUDE_DIR /usr/local/include)
include_directories("${RDKAFKA_INCLUDE_DIR}")

# Protobuf generated "gRPC dial-out" 
set(dialout_pb_cc
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout.pb.cc")
set(dialout_pb_h
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout.pb.h")
set(dialout_pb_grpc_cc
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout.grpc.pb.cc")
set(dialout_pb_grpc_h
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/mdt_dialout.grpc.pb.h")
add_custom_command(
    OUTPUT "${dialout_pb_cc}"
            "${dialout_pb_h}"
            "${dialout_pb_grpc_cc}"
            "${dialout_pb_grpc_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
    DEPENDS "${hw_proto}"
)

# Protobuf generated support "GPB-KV" encoding  
set(telemetry_pb_cc
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/telemetry.pb.cc")
set(telemetry_pb_h
    "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/telementry.pb.h")
add_custom_command(
    OUTPUT "${telemetry_pb_cc}"
            "${telemetry_pb_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
        -I "${telemetry_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${telemetry_proto}"
    DEPENDS "${telemetry_proto}"
)

# Include local header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")


# Compile
foreach(_target "mdt_dialout_collector")
    add_executable(
        ${_target}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${_target}.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mdt_dialout_core.cc"
        ${dialout_pb_cc}
        ${dialout_pb_grpc_cc}
        ${telemetry_pb_cc}
    )
    target_compile_features(${_target} PRIVATE cxx_std_17)
    target_link_libraries(
        ${_target}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        ${JSONCPP_LIBRARIES}
        ${RDKAFKA_CPP_LIBRARIES}
        ${RDKAFKA_LIBRARIES}
    )
endforeach()
