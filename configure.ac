m4_include([VERSION]) #Force reconf on VERSION change
AC_INIT([mdt-dialout-collector],[m4_esyscmd_s(cat VERSION)],[salvatore@cuzzilla.org],[mdt-dialout-collector],[http://github.com/scuzzilla/mdt-dialout-collector])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
LT_PATH_LD
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PKG_PROG_PKG_CONFIG

AUTOCONF_ENV="SHELL=/bin/sh"

LT_INIT
AC_ENABLE_STATIC

# Some useful default flags
CFLAGS="-std=gnu89 -Werror -Wall $CFLAGS"
CXXFLAGS="-Werror -Wall -std=c++17 $CXXFLAGS"
AC_DEFINE([__STDC_FORMAT_MACROS], [], [Description])

# Debug
m4_include([config_m4/debug.m4])

# Check for profiling mode
m4_include([config_m4/profiling.m4])

# Checking libs
m4_include([config_m4/versioning.m4])

PKG_CHECK_MODULES([JSONCPP], [jsoncpp], [],
	[AC_MSG_ERROR([Please install libjsoncpp ])])

PKG_CHECK_MODULES([KAFKA], [rdkafka], [],
	[AC_MSG_ERROR([Please install librdkafka ])])
PKG_CHECK_MODULES([KAFKACPP], [rdkafka++], [],
	[AC_MSG_ERROR([Please install librdkafka++ ])])

PKG_CHECK_MODULES([SPDLOG], [spdlog], [],
	[AC_MSG_ERROR([Please install libspdlog ])])

PKG_CHECK_MODULES([PROTOBUF], [protobuf], [],
	[AC_MSG_ERROR([Please install libgrpc ])])

PKG_CHECK_MODULES([GRPC], [grpc], [],
	[AC_MSG_ERROR([Please install libgrpc ])])

PKG_CHECK_MODULES([GRPCCPP], [grpc++], [],
	[AC_MSG_ERROR([Please install libgrpc++ ])])

# Output files
AC_CONFIG_FILES([
	Makefile
	proto/Makefile
	src/Makefile
	src/bridge/Makefile
	src/cfgWrapper/Makefile
	src/core/Makefile
	src/dataDelivery/Makefile
	src/dataManipulation/Makefile
	src/dataWrapper/Makefile
	src/proto/Makefile
	src/utils/Makefile
])

AC_OUTPUT
