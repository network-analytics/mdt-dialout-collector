#!/bin/sh
# Validate the library package in a FRESH container of the target distro.
#
# Steps:
#   1. Install the .deb/.rpm — catches missing/wrong runtime depends.
#   2. Confirm the .so + header + .pc file land in the expected places.
#   3. Compile the dlopen probe (pkg/validate/library.c) and run it —
#      catches any unresolved transitive deps on system gRPC at runtime.

set -eu

PKG_FILE="${1:?usage: $0 <package-file>}"
. /etc/os-release

case "${ID}" in
    debian|ubuntu)
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq "${PKG_FILE}"
        # gcc + libdl come from build-essential.
        apt-get install -y -qq --no-install-recommends gcc libc6-dev
        ;;
    fedora|rocky|rhel|centos|almalinux)
        if [ "${ID}" != "fedora" ]; then
            dnf install -y -q epel-release >/dev/null
        fi
        dnf install -y -q "${PKG_FILE}"
        dnf install -y -q gcc glibc-devel
        ;;
    *)
        echo "unsupported distro: ${ID}" >&2
        exit 2
        ;;
esac

echo "==> file layout check"
test -f /opt/mdt-dialout-collector/lib/libgrpc_collector.so
test -f /opt/mdt-dialout-collector/include/grpc_collector_bridge/grpc_collector_bridge.h
test -f /opt/mdt-dialout-collector/lib/pkgconfig/grpc_collector.pc

echo "==> pkg-config sanity"
PKG_CONFIG_PATH=/opt/mdt-dialout-collector/lib/pkgconfig \
    pkg-config --modversion grpc-collector >/dev/null 2>&1 || \
PKG_CONFIG_PATH=/opt/mdt-dialout-collector/lib/pkgconfig \
    pkg-config --modversion grpc_collector >/dev/null 2>&1 || true

echo "==> compile dlopen probe"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cc "${SCRIPT_DIR}/library.c" -ldl -o /tmp/lib_validate

echo "==> run dlopen probe"
/tmp/lib_validate

echo
echo "LIBRARY PACKAGE VALIDATION OK (${ID} ${VERSION_ID:-})"
