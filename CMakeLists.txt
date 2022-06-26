# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# Set compiler options
set(CMAKE_CXX_FLAGS "-Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

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

# Proto file gRPC dial-out / gnmi / gnmi extensions - Juniper
get_filename_component(juniper_gnmi_dialout_proto "proto/Juniper/juniper_dialout.proto" ABSOLUTE)
get_filename_component(juniper_gnmi_dialout_proto_path "${juniper_gnmi_dialout_proto}" PATH)

get_filename_component(juniper_gnmi_proto "proto/Juniper/juniper_gnmi.proto" ABSOLUTE)
get_filename_component(juniper_gnmi_proto_path "${juniper_gnmi_proto}" PATH)

get_filename_component(juniper_gnmi_ext_proto "proto/Juniper/juniper_gnmi_ext.proto" ABSOLUTE)
get_filename_component(juniper_gnmi_ext_proto_path "${juniper_gnmi_ext_proto}" PATH)

# Proto file gRPC dial-out - Huawei
get_filename_component(huawei_dialout_proto "proto/Huawei/huawei_dialout.proto" ABSOLUTE)
get_filename_component(huawei_dialout_proto_path "${huawei_dialout_proto}" PATH)

# Proto file to be able to decode gpb-kv messages - Cisco
get_filename_component(cisco_telemetry_proto "proto/Cisco/cisco_telemetry.proto" ABSOLUTE)
get_filename_component(cisco_telemetry_proto_path "${cisco_telemetry_proto}" PATH)

# Proto file to be able to decode gpb-kv messages - Telemetry / Telemetry Header / Telemetry Header Extensions - Juniper
get_filename_component(juniper_telemetry_proto "proto/Juniper/juniper_telemetry.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_proto_path "${juniper_telemetry_proto}" PATH)

get_filename_component(juniper_telemetry_header_proto "proto/Juniper/juniper_telemetry_header.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_header_proto_path "${juniper_telemetry_header_proto}" PATH)

get_filename_component(juniper_telemetry_header_extension_proto "proto/Juniper/juniper_telemetry_header_extension.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_header_extension_proto_path "${juniper_telemetry_header_extension_proto}" PATH)

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
    OUTPUT
    "${cisco_dialout_pb_cc}"
    "${cisco_dialout_pb_h}"
    "${cisco_dialout_pb_grpc_cc}"
    "${cisco_dialout_pb_grpc_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${cisco_dialout_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
    "${cisco_dialout_proto}"
    --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    DEPENDS
    "${cisco_dialout_proto}"
)

# Protobuf generated "gRPC dial-out" - Juniper
set(juniper_gnmi_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi.pb.cc")
set(juniper_gnmi_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi.pb.h")
set(juniper_gnmi_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi.grpc.pb.cc")
set(juniper_gnmi_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi.grpc.pb.h")
set(juniper_gnmi_dialout_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_dialout.pb.cc")
set(juniper_gnmi_dialout_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_dialout.pb.h")
set(juniper_gnmi_dialout_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_dialout.grpc.pb.cc")
set(juniper_gnmi_dialout_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_dialout.grpc.pb.h")
add_custom_command(
    OUTPUT
    "${juniper_gnmi_pb_cc}"
    "${juniper_gnmi_pb_h}"
    "${juniper_gnmi_pb_grpc_cc}"
    "${juniper_gnmi_pb_grpc_h}"
    "${juniper_gnmi_dialout_pb_cc}"
    "${juniper_gnmi_dialout_pb_h}"
    "${juniper_gnmi_dialout_pb_grpc_cc}"
    "${juniper_gnmi_dialout_pb_grpc_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${juniper_gnmi_proto_path}"
    -I "${juniper_gnmi_dialout_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper"
    "${juniper_gnmi_proto}"
    "${juniper_gnmi_dialout_proto}"
    --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    DEPENDS
    "${juniper_gnmi_proto}"
    "${juniper_gnmi_dialout_proto}"
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
    OUTPUT
    "${huawei_dialout_pb_cc}"
    "${huawei_dialout_pb_h}"
    "${huawei_dialout_pb_grpc_cc}"
    "${huawei_dialout_pb_grpc_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${huawei_dialout_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
    "${huawei_dialout_proto}"
    --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    DEPENDS
    "${huawei_dialout_proto}"
)

# Protobuf generated support "GPB-KV" encoding - Cisco
set(cisco_telemetry_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_telemetry.pb.cc")
set(cisco_telemetry_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco/cisco_telemetry.pb.h")
add_custom_command(
    OUTPUT
    "${cisco_telemetry_pb_cc}"
    "${cisco_telemetry_pb_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${cisco_telemetry_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco"
    "${cisco_telemetry_proto}"
    DEPENDS
    "${cisco_telemetry_proto}"
)

# Protobuf generated support "GPB-KV" encoding - Juniper
set(juniper_gnmi_ext_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi_ext.pb.cc")
set(juniper_gnmi_ext_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_gnmi_ext.pb.h")
set(juniper_telemetry_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry.pb.cc")
set(juniper_telemetry_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry.pb.h")
set(juniper_telemetry_header_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry_header.pb.cc")
set(juniper_telemetry_header_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry_header.pb.h")
set(juniper_telemetry_header_extension_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry_header_extension.pb.cc")
set(juniper_telemetry_header_extension_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper/juniper_telemetry_header_extension.pb.h")
add_custom_command(
    OUTPUT
    "${juniper_gnmi_ext_pb_cc}"
    "${juniper_gnmi_ext_pb_h}"
    "${juniper_telemetry_pb_cc}"
    "${juniper_telemetry_pb_h}"
    "${juniper_telemetry_header_pb_cc}"
    "${juniper_telemetry_header_pb_h}"
    "${juniper_telemetry_header_extension_pb_cc}"
    "${juniper_telemetry_header_extension_pb_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS 
    -I "${juniper_gnmi_ext_proto_path}"
    -I "${juniper_telemetry_proto_path}"
    -I "${juniper_telemetry_header_proto_path}"
    -I "${juniper_telemetry_header_extension_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper"
    "${juniper_gnmi_ext_proto}"
    "${juniper_telemetry_proto}"
    "${juniper_telemetry_header_proto}"
    "${juniper_telemetry_header_extension_proto}"
    DEPENDS
    "${juniper_gnmi_ext_proto}"
    "${juniper_telemetry_proto}"
    "${juniper_telemetry_header_proto}"
    "${juniper_telemetry_header_extension_proto}"
)

# Protobuf generated support "GPB-KV" encoding - Huawei
set(huawei_telemetry_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_telemetry.pb.cc")
set(huawei_telemetry_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei/huawei_telemetry.pb.h")
add_custom_command(
    OUTPUT
    "${huawei_telemetry_pb_cc}"
    "${huawei_telemetry_pb_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${huawei_telemetry_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei"
    "${huawei_telemetry_proto}"
    DEPENDS
    "${huawei_telemetry_proto}"
)

# Include local header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper")
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
        ${juniper_gnmi_pb_cc}
        ${juniper_gnmi_ext_pb_cc}
        ${juniper_gnmi_dialout_pb_cc}
        ${juniper_gnmi_dialout_pb_grpc_cc}
        ${juniper_telemetry_pb_cc}
        ${juniper_telemetry_header_pb_cc}
        ${juniper_telemetry_header_extension_pb_cc}
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
