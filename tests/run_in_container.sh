#!/usr/bin/env bash
# Configure + build + run the lightweight unit tests inside a throwaway
# Debian container, driven by tests/CMakeLists.txt as a standalone project.
# This intentionally bypasses the parent CMakeLists.txt so we don't drag in
# the full gRPC/Protobuf-from-source build for cfg_handler-only tests.
# Host stays clean: no dev packages installed locally.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${IMAGE:-debian:bookworm-slim}"

podman run --rm -v "${REPO_ROOT}:/src:ro,Z" -w /src "${IMAGE}" bash -c '
    set -e
    apt-get update -qq >/dev/null
    apt-get install -y -qq \
        cmake g++ make pkg-config \
        libspdlog-dev libfmt-dev libconfig++-dev libgtest-dev \
        >/dev/null

    BUILD=/tmp/build
    mkdir -p ${BUILD}
    cmake -S /src/tests -B ${BUILD}
    cmake --build ${BUILD} -j"$(nproc)"
    ctest --test-dir ${BUILD} --output-on-failure
'
