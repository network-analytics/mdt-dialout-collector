#!/bin/sh
# Install build deps for the current distro. Called from CI before
# build_collector.sh. Detects the distro via /etc/os-release.
#
# Outputs (env vars exported via $GITHUB_ENV when run under GH Actions):
#   DISTRO_TAG       — short tag for artifact filenames (bookworm, trixie,
#                       noble, el9, fc41, ...)
#   GRPC_RUNTIME     — runtime SOVER package name (libgrpc++1.51 / grpc-cpp / ...)
#   PROTOBUF_RUNTIME — runtime SOVER package name (libprotobuf32 / protobuf / ...)
#   PKG_FORMAT       — "deb" or "rpm"

set -eu

. /etc/os-release

case "${ID}" in
    debian|ubuntu)
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y --no-install-recommends \
            build-essential cmake pkg-config git \
            autoconf automake libtool \
            libgrpc++-dev protobuf-compiler-grpc \
            libprotobuf-dev protobuf-compiler \
            libssl-dev libfmt-dev libconfig++-dev \
            librdkafka-dev libjsoncpp-dev libzmq3-dev cppzmq-dev \
            libspdlog-dev \
            >/dev/null
        PKG_FORMAT=deb
        case "${VERSION_CODENAME:-${ID}}" in
            bookworm)
                DISTRO_TAG=bookworm
                GRPC_RUNTIME=libgrpc++1.51
                PROTOBUF_RUNTIME=libprotobuf32
                ;;
            trixie|forky|noble|jammy|oracular|plucky)
                # All t64-ABI distros currently ship libgrpc++1.51t64 + libprotobuf32t64.
                DISTRO_TAG="${VERSION_CODENAME}"
                GRPC_RUNTIME="libgrpc++1.51t64 | libgrpc++1.66t64"
                PROTOBUF_RUNTIME="libprotobuf32t64 | libprotobuf32"
                ;;
            *)
                DISTRO_TAG="${ID}-${VERSION_ID:-unknown}"
                GRPC_RUNTIME=libgrpc++-dev
                PROTOBUF_RUNTIME=libprotobuf-dev
                ;;
        esac
        ;;
    fedora)
        dnf install -y -q \
            gcc-c++ cmake pkgconf-pkg-config git \
            autoconf automake libtool \
            grpc-devel grpc-plugins protobuf-devel protobuf-compiler \
            openssl-devel fmt-devel libconfig-devel \
            librdkafka-devel jsoncpp-devel zeromq-devel cppzmq-devel \
            spdlog-devel \
            >/dev/null
        PKG_FORMAT=rpm
        DISTRO_TAG="fc${VERSION_ID}"
        GRPC_RUNTIME=grpc-cpp
        PROTOBUF_RUNTIME=protobuf
        ;;
    rocky|rhel|centos|almalinux)
        # CRB hosts protobuf-devel/libconfig-devel/librdkafka-devel; EPEL hosts grpc.
        dnf install -y -q dnf-plugins-core >/dev/null
        dnf config-manager --set-enabled crb >/dev/null 2>&1 \
            || dnf config-manager --set-enabled powertools >/dev/null 2>&1 || true
        dnf install -y -q epel-release >/dev/null
        dnf install -y -q \
            gcc-c++ cmake pkgconf-pkg-config git \
            autoconf automake libtool \
            grpc-devel grpc-plugins protobuf-devel protobuf-compiler \
            openssl-devel fmt-devel libconfig-devel \
            librdkafka-devel jsoncpp-devel zeromq-devel cppzmq-devel \
            spdlog-devel \
            >/dev/null
        PKG_FORMAT=rpm
        DISTRO_TAG="el${VERSION_ID%%.*}"
        GRPC_RUNTIME=grpc-cpp
        PROTOBUF_RUNTIME=protobuf
        ;;
    *)
        echo "unsupported distro: ${ID}" >&2
        exit 2
        ;;
esac

# Export to GitHub Actions env, plus stdout for local debugging.
{
    echo "DISTRO_TAG=${DISTRO_TAG}"
    echo "PKG_FORMAT=${PKG_FORMAT}"
    echo "GRPC_RUNTIME=${GRPC_RUNTIME}"
    echo "PROTOBUF_RUNTIME=${PROTOBUF_RUNTIME}"
} | tee -a "${GITHUB_ENV:-/dev/null}"
