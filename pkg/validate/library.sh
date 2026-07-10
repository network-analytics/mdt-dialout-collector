#!/bin/sh
# Validate the library package in a FRESH container of the target distro.
#
# Steps:
#   1. Install the .deb/.rpm — catches missing/wrong runtime depends.
#   2. Confirm the .so + header + .pc file land in the expected places.
#   3. pkg-config must resolve the module exactly the way pmacct's
#      configure.ac asks for it: grpc-collector >= v1.0.0.
#   4. Compile the dlopen probe (pkg/validate/library.c) and run it —
#      catches any unresolved transitive deps on system gRPC at runtime.
#   5. Compile + run the link probe (pkg/validate/library-link.c) with the
#      pkg-config Cflags/Libs — proves the header include path, -l link and
#      runtime loader resolution via ld.so.conf.d all work like pmacct's.

set -eu

PKG_FILE="${1:?usage: $0 <package-file>}"
. /etc/os-release

# apt-get treats foo/bar.deb as "release/package"; local file install needs ./
case "${PKG_FILE}" in /*|./*) ;; *) PKG_FILE="./${PKG_FILE}" ;; esac

case "${ID}" in
    debian|ubuntu)
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq "${PKG_FILE}"
        # gcc + libdl come from build-essential; zmq.h is included by the bridge header.
        apt-get install -y -qq --no-install-recommends gcc libc6-dev pkg-config libzmq3-dev
        ;;
    fedora|rocky|rhel|centos|almalinux)
        if [ "${ID}" != "fedora" ]; then
            dnf install -y -q epel-release >/dev/null
        fi
        dnf install -y -q "${PKG_FILE}"
        dnf install -y -q gcc glibc-devel pkgconf-pkg-config zeromq-devel
        ;;
    *)
        echo "unsupported distro: ${ID}" >&2
        exit 2
        ;;
esac

echo "==> file layout check"
test -f /opt/mdt-dialout-collector/lib/libgrpc_collector.so.0
test -L /opt/mdt-dialout-collector/lib/libgrpc_collector.so
test -f /opt/mdt-dialout-collector/include/grpc_collector_bridge/grpc_collector_bridge.h
test -f /opt/mdt-dialout-collector/lib/pkgconfig/grpc-collector.pc
test -f /etc/ld.so.conf.d/mdt-dialout-collector-lib.conf

echo "==> loader cache check (postinstall ldconfig)"
ldconfig -p | grep -F libgrpc_collector.so.0

export PKG_CONFIG_PATH=/opt/mdt-dialout-collector/lib/pkgconfig

echo "==> pkg-config module + version check (pmacct's configure.ac form)"
pkg-config --modversion grpc-collector
pkg-config --exists --print-errors 'grpc-collector >= v1.0.0'

echo "==> compile dlopen probe"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cc "${SCRIPT_DIR}/library.c" -ldl -o /tmp/lib_validate

echo "==> run dlopen probe"
/tmp/lib_validate

echo "==> compile link probe (pkg-config Cflags/Libs, like pmacct)"
# shellcheck disable=SC2046 — pkg-config output is intentionally word-split
cc "${SCRIPT_DIR}/library-link.c" \
    $(pkg-config --cflags grpc-collector) \
    $(pkg-config --libs grpc-collector) -o /tmp/lib_link_validate

echo "==> run link probe (loader must resolve libgrpc_collector.so.0)"
/tmp/lib_link_validate

echo
echo "LIBRARY PACKAGE VALIDATION OK (${ID} ${VERSION_ID:-})"
