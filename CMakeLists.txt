# Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


# Minimum CMake required
cmake_minimum_required(VERSION 3.16)

# Set compiler options
#set(CMAKE_C_FLAGS "-O2 -g -Werror -Wall -std=gnu89")
set(CMAKE_CXX_FLAGS "-O2 -g -Wall -pedantic -pthread -std=c++17")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Proto file gRPC dial-out / gnmi / gnmi extensions - Nokia
get_filename_component(nokia_gnmi_dialout_proto "proto/Nokia/nokia_dialout.proto" ABSOLUTE)
get_filename_component(nokia_gnmi_dialout_proto_path "${nokia_gnmi_dialout_proto}" PATH)

get_filename_component(nokia_gnmi_proto "proto/Nokia/nokia_gnmi.proto" ABSOLUTE)
get_filename_component(nokia_gnmi_proto_path "${nokia_gnmi_proto}" PATH)

get_filename_component(nokia_gnmi_ext_proto "proto/Nokia/nokia_gnmi_ext.proto" ABSOLUTE)
get_filename_component(nokia_gnmi_ext_proto_path "${nokia_gnmi_ext_proto}" PATH)

# Proto file gRPC dial-out - Huawei
get_filename_component(huawei_dialout_proto "proto/Huawei/huawei_dialout.proto" ABSOLUTE)
get_filename_component(huawei_dialout_proto_path "${huawei_dialout_proto}" PATH)

# Proto file to be able to decode gpb-kv messages - Cisco
get_filename_component(cisco_telemetry_proto "proto/Cisco/cisco_telemetry.proto" ABSOLUTE)
get_filename_component(cisco_telemetry_proto_path "${cisco_telemetry_proto}" PATH)

# Proto file to be able to decode gpb messages - Telemetry / Telemetry Header / Telemetry Header Extensions - Juniper
get_filename_component(juniper_telemetry_proto "proto/Juniper/juniper_telemetry.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_proto_path "${juniper_telemetry_proto}" PATH)

get_filename_component(juniper_telemetry_header_proto "proto/Juniper/juniper_telemetry_header.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_header_proto_path "${juniper_telemetry_header_proto}" PATH)

get_filename_component(juniper_telemetry_header_extension_proto "proto/Juniper/juniper_telemetry_header_extension.proto" ABSOLUTE)
get_filename_component(juniper_telemetry_header_extension_proto_path "${juniper_telemetry_header_extension_proto}" PATH)

# Proto file to be able to decode gpb messages - Huawei
get_filename_component(huawei_telemetry_proto "proto/Huawei/huawei_telemetry.proto" ABSOLUTE)
get_filename_component(huawei_telemetry_proto_path "${huawei_telemetry_proto}" PATH)

# GPB "Subject" specific, Vendors & Standardization entities
# Cisco

# Juniper

# Huawei

# OpenConfig
get_filename_component(openconfig_interfaces_proto "proto/OpenConfig/openconfig_interfaces.proto" ABSOLUTE)
get_filename_component(openconfig_interfaces_proto_path "${openconfig_interfaces_proto}" PATH)

# jsoncpp libs
set(JSONCPP_INCLUDE_DIR /usr/include/jsoncpp)
include_directories("${JSONCPP_INCLUDE_DIR}")
pkg_check_modules(JSONCPP REQUIRED jsoncpp>=1.8.4)
link_libraries(${JSONCPP_LIBRARIES})
message(STATUS "Using jsoncpp ${JSONCPP_VERSION}")

# librdkafka (MANUAL == manually compiled)
set(RDKAFKACPP_INCLUDE_DIR /usr/include/librdkafka)
#set(RDKAFKACPP_INCLUDE_DIR_MANUAL /usr/local/include)
include_directories("${RDKAFKACPP_INCLUDE_DIR}")
#include_directories("${RDKAFKACPP_INCLUDE_DIR_MANUAL}")
pkg_check_modules(RDKAFKA_CPP REQUIRED rdkafka++>=1.6.0)
link_libraries(${RDKAFKA_CPP_LIBRARIES})
message(STATUS "Using librdkafka++ ${RDKAFKA_CPP_VERSION}")

set(RDKAFKA_INCLUDE_DIR /usr/include/librdkafka)
#set(RDKAFKA_INCLUDE_DIR_MANUAL /usr/local/include)
include_directories("${RDKAFKA_INCLUDE_DIR}")
#include_directories("${RDKAFKA_INCLUDE_DIR_MANUAL}")
pkg_check_modules(RDKAFKA REQUIRED rdkafka>=1.6.0)
link_libraries(${RDKAFKA_LIBRARIES})
message(STATUS "Using librdkafka ${RDKAFKA_VERSION}")

# CPPZMQ (https://github.com/zeromq/cppzmq)
set(CPPZMQ_INCLUDE_DIR /usr/include)
include_directories("${CPPZMQ_INCLUDE_DIR}")
pkg_check_modules(ZMQ_CPP REQUIRED libzmq>=4.3.2)
link_libraries(${ZMQ_CPP_LIBRARIES})
message(STATUS "Using cppzmq ${ZMQ_CPP_VERSION}")

# libconfig
set(LIBCONFIG_CPP_INCLUDE_DIR /usr/include)
include_directories("${LIBCONFIG_CPP_INCLUDE_DIR}")
pkg_check_modules(LIBCONFIG_CPP libconfig++>=1.5)
link_libraries(${LIBCONFIG_CPP_LIBRARIES})
message(STATUS "Using libconfig++ ${LIBCONFIG_CPP_VERSION}")

# spdlog
set(SPDLOG_INCLUDE_DIR /usr/include/spdlog)
include_directories("${SPDLOG_INCLUDE_DIR}")
pkg_check_modules(SPDLOG spdlog>=1.5.0)
link_libraries(${SPDLOG_LIBRARIES})
message(STATUS "Using spdlog ${SPDLOG_VERSION}")

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

# Protobuf generated "gRPC dial-out" - Nokia
set(nokia_gnmi_ext_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi_ext.pb.cc")
set(nokia_gnmi_ext_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi_ext.pb.h")
set(nokia_gnmi_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi.pb.cc")
set(nokia_gnmi_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi.pb.h")
set(nokia_gnmi_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi.grpc.pb.cc")
set(nokia_gnmi_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_gnmi.grpc.pb.h")
set(nokia_gnmi_dialout_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_dialout.pb.cc")
set(nokia_gnmi_dialout_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_dialout.pb.h")
set(nokia_gnmi_dialout_pb_grpc_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_dialout.grpc.pb.cc")
set(nokia_gnmi_dialout_pb_grpc_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia/nokia_dialout.grpc.pb.h")
add_custom_command(
    OUTPUT
    "${nokia_gnmi_ext_pb_cc}"
    "${nokia_gnmi_ext_pb_h}"
    "${nokia_gnmi_pb_cc}"
    "${nokia_gnmi_pb_h}"
    "${nokia_gnmi_pb_grpc_cc}"
    "${nokia_gnmi_pb_grpc_h}"
    "${nokia_gnmi_dialout_pb_cc}"
    "${nokia_gnmi_dialout_pb_h}"
    "${nokia_gnmi_dialout_pb_grpc_cc}"
    "${nokia_gnmi_dialout_pb_grpc_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${nokia_gnmi_ext_proto_path}"
    -I "${nokia_gnmi_proto_path}"
    -I "${nokia_gnmi_dialout_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia"
    "${nokia_gnmi_ext_proto}"
    "${nokia_gnmi_proto}"
    "${nokia_gnmi_dialout_proto}"
    --grpc_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Nokia"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    DEPENDS
    "${nokia_gnmi_ext_proto}"
    "${nokia_gnmi_proto}"
    "${nokia_gnmi_dialout_proto}"
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

# Protobuf generated support "GPB-KV" decoding - Cisco
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

# Protobuf generated support "GPB" decoding - Juniper
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

# Protobuf generated support "GPB" decoding - Huawei
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

# Protobuf generated support "GPB" decoding - Openconfig
set(openconfig_interfaces_pb_cc
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/OpenConfig/openconfig_interfaces.pb.cc")
set(openconfig_interfaces_pb_h
  "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/OpenConfig/openconfig_interfaces.pb.h")
add_custom_command(
    OUTPUT
    "${openconfig_interfaces_pb_cc}"
    "${openconfig_interfaces_pb_h}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    -I "${openconfig_interfaces_proto_path}"
    --cpp_out "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/OpenConfig"
    "${openconfig_interfaces_proto}"
    DEPENDS
    "${openconfig_interfaces_proto}"
)

# Include headers folders | Relative paths to allow autotools coexistence
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Cisco")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Juniper")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/Huawei")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/proto/OpenConfig")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/core")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/utils")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/dataManipulation")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/dataWrapper")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/cfgWrapper")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/dataDelivery")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/bridge")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# set the bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Compile CC
file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
foreach(sourcefile ${APP_SOURCES})
    file(RELATIVE_PATH filename ${CMAKE_CURRENT_SOURCE_DIR}/src ${sourcefile})
    string(REPLACE ".cc" "" file ${filename})
    add_executable(
        ${file}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${file}.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/mdt_dialout_core.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/cfg_handler.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/logs_handler.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataManipulation/data_manipulation.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataWrapper/data_wrapper.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cfgWrapper/cfg_wrapper.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataDelivery/kafka_delivery.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataDelivery/zmq_delivery.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/grpc_collector_bridge.cc"
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
        ${nokia_gnmi_pb_cc}
        ${nokia_gnmi_ext_pb_cc}
        ${nokia_gnmi_dialout_pb_cc}
        ${nokia_gnmi_dialout_pb_grpc_cc}
        ${huawei_dialout_pb_cc}
        ${huawei_dialout_pb_grpc_cc}
        ${huawei_telemetry_pb_cc}
        ${openconfig_interfaces_pb_cc}
    )
    target_compile_features(
        ${file}
        PRIVATE
    )
    target_link_libraries(
        ${file}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        ${JSONCPP_LIBRARIES}
        ${RDKAFKA_CPP_LIBRARIES}
        ${RDKAFKA_LIBRARIES}
        ${LIBCONFIG_CPP_LIBRARIES}
    )
endforeach(sourcefile ${APP_SOURCES})

# Compile C
#file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
#foreach(sourcefile ${APP_SOURCES})
#    file(RELATIVE_PATH filename ${CMAKE_CURRENT_SOURCE_DIR}/src ${sourcefile})
#    string(REPLACE ".c" "" file ${filename})
#    add_executable(
#        ${file}
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/${file}.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/mdt_dialout_core.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/cfg_handler.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/logs_handler.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataManipulation/data_manipulation.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataWrapper/data_wrapper.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/cfgWrapper/cfg_wrapper.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataDelivery/kafka_delivery.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/dataDelivery/zmq_delivery.cc"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/bridge/grpc_collector_bridge.cc"
#        ${cisco_dialout_pb_cc}
#        ${cisco_dialout_pb_grpc_cc}
#        ${cisco_telemetry_pb_cc}
#        ${juniper_gnmi_pb_cc}
#        ${juniper_gnmi_ext_pb_cc}
#        ${juniper_gnmi_dialout_pb_cc}
#        ${juniper_gnmi_dialout_pb_grpc_cc}
#        ${juniper_telemetry_pb_cc}
#        ${juniper_telemetry_header_pb_cc}
#        ${juniper_telemetry_header_extension_pb_cc}
#        ${huawei_dialout_pb_cc}
#        ${huawei_dialout_pb_grpc_cc}
#        ${huawei_telemetry_pb_cc}
#        ${openconfig_interfaces_pb_cc}
#    )
#    target_compile_features(
#        ${file}
#        PRIVATE
#    )
#    target_link_libraries(
#        ${file}
#        ${_REFLECTION}
#        ${_GRPC_GRPCPP}
#        ${_PROTOBUF_LIBPROTOBUF}
#        ${JSONCPP_LIBRARIES}
#        ${RDKAFKA_CPP_LIBRARIES}
#        ${RDKAFKA_LIBRARIES}
#        ${LIBCONFIG_CPP_LIBRARIES}
#    )
#endforeach(sourcefile ${APP_SOURCES})
