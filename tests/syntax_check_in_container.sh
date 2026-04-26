#!/usr/bin/env bash
# Heavy-weight syntax-only build check for files whose deps require the full
# gRPC + protobuf stack. Intentionally separate from run_in_container.sh so
# the lightweight unit-test loop stays fast.
#
# Re-generates the .pb.h/.pb.cc into a tmp dir inside the container so the
# host's stale src/proto/* files are not used.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${IMAGE:-debian:bookworm-slim}"

# Add files here as fixes touch other heavy units.
FILES_TO_CHECK=(
    "src/core/mdt_dialout_core.cc"
)

podman run --rm -v "${REPO_ROOT}:/src:ro,Z" -w /src "${IMAGE}" bash -c "
    set -e
    apt-get update -qq >/dev/null
    apt-get install -y -qq \
        g++ \
        libgrpc++-dev libprotobuf-dev \
        protobuf-compiler protobuf-compiler-grpc \
        libjsoncpp-dev libspdlog-dev libfmt-dev \
        libconfig++-dev libzmq3-dev cppzmq-dev \
        librdkafka-dev >/dev/null

    GEN=/tmp/gen
    mkdir -p \${GEN}/proto/{Cisco,Juniper,Nokia,Huawei,OpenConfig}
    PLUGIN=\$(which grpc_cpp_plugin)
    for v in Cisco Juniper Nokia Huawei OpenConfig; do
        [ -d proto/\${v} ] || continue
        for p in proto/\${v}/*.proto; do
            protoc -I proto/\${v} \
                --cpp_out=\${GEN}/proto/\${v} \
                --grpc_out=\${GEN}/proto/\${v} \
                --plugin=protoc-gen-grpc=\${PLUGIN} \
                \${p}
        done
    done

    fail=0
    for f in ${FILES_TO_CHECK[*]}; do
        echo === syntax-check \${f} ===
        # Fresh \${GEN} comes BEFORE -I src so freshly-generated headers
        # win over stale src/proto/*.pb.h shipped in the tree.
        if g++ -fsyntax-only -std=c++17 -O2 -g -Wall -pedantic -pthread \
            -I \${GEN} -I src/include -I src \
            -I /usr/include/jsoncpp -I /usr/include/librdkafka \
            -c \${f}; then
            echo OK: \${f}
        else
            echo FAIL: \${f}
            fail=1
        fi
    done
    exit \${fail}
"
