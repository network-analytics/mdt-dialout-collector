# Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
# Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

# syntax=docker/dockerfile:1
#
# Multi-target image: two final stages mirror the .deb/.rpm pair.
#   --target standalone → daemon binary (replaces old monolithic image)
#   --target library    → libgrpc_collector.so + bridge header + .pc file,
#                         intended as a base for pmtelemetryd container builds
#
# Build context MUST be the repository root (CI passes `.`); the COPYs below
# pull source from the checkout, not from a git clone inside the container.

ARG BASE=debian:12

# --- shared build-deps stage ----------------------------------------------
FROM ${BASE} AS build-base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        autoconf automake libtool \
        libgrpc++-dev protobuf-compiler-grpc \
        libprotobuf-dev protobuf-compiler \
        libssl-dev libfmt-dev libconfig++-dev \
        librdkafka-dev libjsoncpp-dev libzmq3-dev cppzmq-dev \
        libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

# --- standalone build (cmake → bin/mdt_dialout_collector) -----------------
FROM build-base AS build-standalone
WORKDIR /src
COPY CMakeLists.txt VERSION ./
COPY proto ./proto
COPY src ./src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j"$(nproc)"

# --- library build (autotools → src/.libs/libgrpc_collector.so) -----------
FROM build-base AS build-library
WORKDIR /src
COPY VERSION configure.ac autogen.sh Makefile.am grpc-collector.pc.in ./
COPY config_m4 ./config_m4
COPY proto ./proto
COPY src ./src
RUN ./autogen.sh \
    && ./configure --prefix=/opt/mdt-dialout-collector --quiet \
    && make -j"$(nproc)"
# Render the .pc file from the .pc.in template.
RUN mkdir -p /out/lib/pkgconfig \
    && VER="$(sed 's/^v//' VERSION)" \
    && sed -e "s|@prefix@|/opt/mdt-dialout-collector|g" \
           -e "s|@exec_prefix@|/opt/mdt-dialout-collector|g" \
           -e "s|@libdir@|/opt/mdt-dialout-collector/lib|g" \
           -e "s|@includedir@|/opt/mdt-dialout-collector/include|g" \
           -e "s|@VERSION@|${VER}|g" \
           grpc-collector.pc.in > /out/lib/pkgconfig/grpc_collector.pc

# --- common runtime base (Debian 12 SOVER pinned via BASE arg) ------------
FROM ${BASE} AS runtime-base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && apt-get install -y --no-install-recommends \
        libgrpc++1.51 libprotobuf32 \
        libssl3 libfmt9 libconfig++9v5 \
        librdkafka1 librdkafka++1 libjsoncpp25 libzmq5 libspdlog1.10 \
    && rm -rf /var/lib/apt/lists/*

# --- standalone runtime image ---------------------------------------------
FROM runtime-base AS standalone
COPY --from=build-standalone /src/bin/mdt_dialout_collector \
                             /opt/mdt-dialout-collector/bin/mdt_dialout_collector
COPY docker/scripts/mdt_dialout_collector.conf \
     /etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf
ENTRYPOINT ["/opt/mdt-dialout-collector/bin/mdt_dialout_collector"]
CMD ["-f", "/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf"]

# --- library runtime image -------------------------------------------------
# Files-only image: the .so + header + pkg-config land under /opt/. Intended
# as a FROM base for pmtelemetryd-style container builds. The default CMD
# prints the layout so `docker run` is at least informative.
FROM runtime-base AS library
COPY --from=build-library /src/src/.libs/libgrpc_collector.so \
     /opt/mdt-dialout-collector/lib/libgrpc_collector.so
COPY --from=build-library /src/src/bridge/grpc_collector_bridge.h \
     /opt/mdt-dialout-collector/include/grpc_collector_bridge/grpc_collector_bridge.h
COPY --from=build-library /out/lib/pkgconfig/grpc_collector.pc \
     /opt/mdt-dialout-collector/lib/pkgconfig/grpc_collector.pc
CMD ["sh", "-c", "echo 'mdt-dialout-collector-lib — layout:'; ls -laR /opt/mdt-dialout-collector"]
