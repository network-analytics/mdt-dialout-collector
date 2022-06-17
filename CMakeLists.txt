# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# pkg-config
find_package(PkgConfig REQUIRED)

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

# Proto file gRPC dial-out - Cisco
get_filename_component(cisco_dialout_proto "proto/Cisco/cisco_dialout.proto" ABSOLUTE)
get_filename_component(cisco_dialout_proto_path "${cisco_dialout_proto}" PATH)

# Proto file gRPC dial-out - Huawei
get_filename_component(huawei_dialout_proto "proto/Huawei/huawei_dialout.proto" ABSOLUTE)
get_filename_component(huawei_dialout_proto_path "${huawei_dialout_proto}" PATH)

# Proto file to be able to decode gpb-kv messages - Cisco
get_filename_component(cisco_telemetry_proto "proto/Cisco/cisco_telemetry.proto" ABSOLUTE)
get_filename_component(cisco_telemetry_proto_path "${cisco_telemetry_proto}" PATH)

# Proto file to be able to decode gpb-kv messages - Huawei
get_filename_component(huawei_telemetry_proto "proto/Huawei/huawei_telemetry.proto" ABSOLUTE)
get_filename_component(huawei_telemetry_proto_path "${huawei_telemetry_proto}" PATH)

# jsoncpp libs
set(JSONCPP_INCLUDE_DIR /usr/include/jsoncpp)
include_directories("${JSONCPP_INCLUDE_DIR}")
pkg_check_modules(JSONCPP REQUIRED jsoncpp>=1.8.4)
link_libraries(${JSONCPP_LIBRARIES})
message(STATUS "Using jsoncpp ${JSONCPP_VERSION}")

# librdkafka
set(RDKAFKACPP_INCLUDE_DIR /usr/local/include)
include_directories("${RDKAFKACPP_INCLUDE_DIR}")
pkg_check_modules(RDKAFKA_CPP REQUIRED rdkafka++>=1.6.2)
link_libraries(${RDKAFKA_CPP_LIBRARIES})
message(STATUS "Using librdkafka++ ${RDKAFKA_CPP_VERSION}")

set(RDKAFKA_INCLUDE_DIR /usr/local/include)
include_directories("${RDKAFKA_INCLUDE_DIR}")
pkg_check_modules(RDKAFKA REQUIRED rdkafka>=1.6.2)
link_libraries(${RDKAFKA_LIBRARIES})
message(STATUS "Using librdkafka ${RDKAFKA_VERSION}")

# libconfig
set(LIBCONFIG_CPP_INCLUDE_DIR /usr/include)
include_directories("${LIBCONFIG_CPP_INCLUDE_DIR}")
pkg_check_modules(LIBCONFIG_CPP libconfig++>=1.5)
link_libraries(${LIBCONFIG_CPP_LIBRARIES})
message(STATUS "Using libconfig++ ${LIBCONFIG_CPP_VERSION}")

# Protobuf generated "gRPC dial-out" - Cisco
set(cisco_dialout_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_dialout.pb.cc")
set(cisco_dialout_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_dialout.pb.h")
set(cisco_dialout_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_dialout.grpc.pb.cc")
set(cisco_dialout_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_dialout.grpc.pb.h")
add_custom_command(
    OUTPUT "${cisco_dialout_pb_cc}"
            "${cisco_dialout_pb_h}"
            "${cisco_dialout_pb_grpc_cc}"
            "${cisco_dialout_pb_grpc_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
        -I "${cisco_dialout_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cisco_dialout_proto}"
    DEPENDS "${cisco_dialout_proto}"
)

# Protobuf generated "gRPC dial-out" - Huawei
set(huawei_dialout_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_dialout.pb.cc")
set(huawei_dialout_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_dialout.pb.h")
set(huawei_dialout_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_dialout.grpc.pb.cc")
set(huawei_dialout_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_dialout.grpc.pb.h")
add_custom_command(
    OUTPUT "${huawei_dialout_pb_cc}"
            "${huawei_dialout_pb_h}"
            "${huawei_dialout_pb_grpc_cc}"
            "${huawei_dialout_pb_grpc_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
        -I "${huawei_dialout_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${huawei_dialout_proto}"
    DEPENDS "${huawei_dialout_proto}"
)

# Protobuf generated support "GPB-KV" encoding - Cisco
set(cisco_telemetry_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_telemetry.pb.cc")
set(cisco_telemetry_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_telementry.pb.h")
add_custom_command(
    OUTPUT "${cisco_telemetry_pb_cc}"
            "${cisco_telemetry_pb_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
        -I "${cisco_telemetry_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${cisco_telemetry_proto}"
    DEPENDS "${cisco_telemetry_proto}"
)

# Protobuf generated support "GPB-KV" encoding - Huawei
set(huawei_telemetry_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_telemetry.pb.cc")
set(huawei_telemetry_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_telementry.pb.h")
add_custom_command(
    OUTPUT "${huawei_telemetry_pb_cc}"
            "${huawei_telemetry_pb_h}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
        -I "${huawei_telemetry_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${huawei_telemetry_proto}"
    DEPENDS "${huawei_telemetry_proto}"
)

# Include local header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/utils")

# set the bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Compile
foreach(_target "mdt_dialout_collector")
    add_executable(
        ${_target}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${_target}.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mdt_dialout_core.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/cfg_handler.cc"
        ${cisco_dialout_pb_cc}
        ${cisco_dialout_pb_grpc_cc}
        ${cisco_telemetry_pb_cc}
        ${huawei_dialout_pb_cc}
        ${huawei_dialout_pb_grpc_cc}
        ${huawei_telemetry_pb_cc}
    )
    target_compile_features(${_target} PRIVATE cxx_std_20)
    target_link_libraries(
        ${_target}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        ${JSONCPP_LIBRARIES}
        ${RDKAFKA_CPP_LIBRARIES}
        ${RDKAFKA_LIBRARIES}
        ${LIBCONFIG_CPP_LIBRARIES}
    )
endforeach()
