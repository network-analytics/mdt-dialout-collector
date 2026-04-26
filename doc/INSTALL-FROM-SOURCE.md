# Building mdt-dialout-collector from source

For most operators, [the packaged install](INSTALL.md) is the right
path. This document is for:

- Developers contributing to the project.
- Operators on a distro that's not in the
  [release matrix](../.github/workflows/release.yml).
- Anyone wanting to validate a tip-of-main build.

## Build deps (Debian 12+ / Ubuntu 24.04+)

```sh
sudo apt install \
    build-essential cmake pkg-config git \
    libgrpc++-dev protobuf-compiler-grpc \
    libprotobuf-dev protobuf-compiler \
    libssl-dev libfmt-dev libconfig++-dev \
    librdkafka-dev libjsoncpp-dev libzmq3-dev cppzmq-dev \
    libspdlog-dev
```

## Build deps (Fedora / Rocky 9 + EPEL)

```sh
# Rocky 9 / RHEL 9: gRPC lives in EPEL.
sudo dnf install epel-release

sudo dnf install \
    gcc-c++ cmake pkgconf-pkg-config git \
    grpc-devel grpc-plugins protobuf-devel protobuf-compiler \
    openssl-devel fmt-devel libconfig-devel \
    librdkafka-devel jsoncpp-devel zeromq-devel cppzmq-devel \
    spdlog-devel
```

## Build the standalone binary (CMake)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
sudo cp bin/mdt_dialout_collector /usr/local/bin/
```

## Build the library variant (Autotools)

```sh
./autogen.sh
./configure
make -j"$(nproc)"
sudo make install
```

The library lands at `/usr/local/lib/libgrpc_collector.so` and the C
bridge header at `/usr/local/include/grpc_collector_bridge/`.

## End-to-end tests

The project ships a podman-based e2e harness under
[tests/e2e/](../tests/e2e/) that builds the collector in a container,
spins up redpanda, runs synthetic per-vendor gRPC clients and asserts
the canaries land in the kafka topic. Variants:

```sh
bash tests/e2e/run.sh                  # standalone, IPv4, plaintext
bash tests/e2e/run.sh --ipv6           # IPv6 listening sockets
bash tests/e2e/run.sh --tls            # server-side TLS
bash tests/e2e/run.sh --next           # newer gRPC/protobuf stack
bash tests/e2e/run.sh --pmtelemetryd   # library mode through pmacct
```

## Unit tests (gtest)

```sh
bash tests/run_in_container.sh
```

Runs the cfg_handler + logs_handler + peer_parser fixtures in a
throwaway debian:bookworm-slim container.

## Legacy install.sh

[`install.sh`](../install.sh) builds gRPC v1.45.2 from source under
`$HOME/grpc` and then runs the autotools build. It still works for the
distros it knows about (debian 11/12, ubuntu 20.04–23.04, centos/rocky/
rhel 8–9.2) but it's no longer the recommended path:

- It pins gRPC to 2022-era v1.45.2 by building from source.
- The distro-detection matrix has to be hand-updated for every new
  release.
- It cannot uninstall, upgrade, or rollback the way `apt` / `dnf` can.

Use it only if you specifically need its workflow.
