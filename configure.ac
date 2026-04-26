m4_include([VERSION]) #Force reconf on VERSION change

AC_INIT(
  [mdt-dialout-collector],
  [m4_esyscmd_s(cat VERSION)],
  [salvatore@cuzzilla.org],
  [mdt-dialout-collector],
  [http://github.com/network-analytics/mdt-dialout-collector])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT([/usr/local])

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

# Some useful default flags. -Werror was previously enabled here, but a
# pre-existing -Wuninitialized false-positive on std::atomic<bool>::load
# in the vendored src/include/kafka/Log.h fires on gcc 11+ and breaks the
# library build on every modern distro. CMake's branch never set -Werror,
# so this only ever bit autotools (-l). Warnings still print; act on them.
CFLAGS="-Wall -std=gnu89 $CFLAGS"
CXXFLAGS="-Wall -std=c++17 $CXXFLAGS"

AC_ARG_WITH(
  [pkgconfigdir],
  [AS_HELP_STRING([--with-pkgconfigdir],
  [pkg-config directory to install the .pc file.])],
  [ with_pkgconfigdir=$with_pkgconfigdir ],
  [ with_pkgconfigdir=$libdir/pkgconfig ])

PKGCONFIG_USER=$with_pkgconfigdir
AC_SUBST([PKGCONFIG_USER])

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

# Discover grpc_cpp_plugin in PATH (system pkgs land it under /usr/bin); fall
# back to the legacy install.sh location if PATH lookup fails.
AC_PATH_PROG([GRPC_CPP_PLUGIN], [grpc_cpp_plugin], [/usr/local/bin/grpc_cpp_plugin])
AC_SUBST([GRPC_CPP_PLUGIN])

# Aggregate pkg-config CFLAGS for every per-module Makefile (system jsoncpp on
# Debian lives under /usr/include/jsoncpp, etc — absent from default CPATH).
COLLECTOR_PKG_CFLAGS="$JSONCPP_CFLAGS $KAFKA_CFLAGS $KAFKACPP_CFLAGS $SPDLOG_CFLAGS $PROTOBUF_CFLAGS $GRPC_CFLAGS $GRPCCPP_CFLAGS"
AC_SUBST([COLLECTOR_PKG_CFLAGS])

# Output files
AC_CONFIG_FILES([
  Makefile
  proto/Makefile
  src/Makefile
  src/bridge/Makefile
  src/core/Makefile
  src/dataDelivery/Makefile
  src/dataManipulation/Makefile
  src/dataWrapper/Makefile
  src/proto/Makefile
  src/utils/Makefile
  grpc-collector.pc
])

AC_OUTPUT
