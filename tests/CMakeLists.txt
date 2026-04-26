# Standalone-buildable so the lightweight test loop can bypass the parent's
# heavy gRPC/Protobuf config. Also works when added via parent's
# add_subdirectory(tests) — the conditional project() guard handles both modes.
cmake_minimum_required(VERSION 3.16)
if (NOT DEFINED PROJECT_NAME)
    project(mdt_dialout_collector_tests CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

find_package(PkgConfig REQUIRED)
find_package(GTest CONFIG REQUIRED)
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(LIBCONFIG_CPP REQUIRED libconfig++)

set(_PROJ_SRC "${CMAKE_CURRENT_LIST_DIR}/../src")

# Tests that exercise only cfg_handler.cc.
set(_CFG_TESTS
    test_kafka_ssl_default
    test_logs_cfg
    test_main_cfg
    test_data_manipulation_cfg
    test_kafka_cfg
    test_zmq_cfg
)
foreach(_t IN LISTS _CFG_TESTS)
    add_executable(${_t}
        unit/${_t}.cc
        ${_PROJ_SRC}/utils/cfg_handler.cc
    )
    target_include_directories(${_t} PRIVATE
        ${_PROJ_SRC}/include
        ${_PROJ_SRC}
        unit
        ${SPDLOG_INCLUDE_DIRS}
        ${LIBCONFIG_CPP_INCLUDE_DIRS}
    )
    target_link_libraries(${_t} PRIVATE
        GTest::gtest GTest::gtest_main
        ${SPDLOG_LIBRARIES}
        ${LIBCONFIG_CPP_LIBRARIES}
    )
endforeach()

# LogsHandler test additionally needs logs_handler.cc.
add_executable(test_logs_handler
    unit/test_logs_handler.cc
    ${_PROJ_SRC}/utils/cfg_handler.cc
    ${_PROJ_SRC}/utils/logs_handler.cc
)
target_include_directories(test_logs_handler PRIVATE
    ${_PROJ_SRC}/include
    ${_PROJ_SRC}
    unit
    ${SPDLOG_INCLUDE_DIRS}
    ${LIBCONFIG_CPP_INCLUDE_DIRS}
)
target_link_libraries(test_logs_handler PRIVATE
    GTest::gtest GTest::gtest_main
    ${SPDLOG_LIBRARIES}
    ${LIBCONFIG_CPP_LIBRARIES}
)

# PeerParser is standalone — no deps beyond stdlib.
add_executable(test_peer_parser
    unit/test_peer_parser.cc
    ${_PROJ_SRC}/utils/peer_parser.cc
)
target_include_directories(test_peer_parser PRIVATE
    ${_PROJ_SRC}
)
target_link_libraries(test_peer_parser PRIVATE
    GTest::gtest GTest::gtest_main
)

enable_testing()
foreach(_t IN LISTS _CFG_TESTS)
    add_test(NAME ${_t} COMMAND ${_t})
endforeach()
add_test(NAME test_logs_handler COMMAND test_logs_handler)
add_test(NAME test_peer_parser  COMMAND test_peer_parser)
