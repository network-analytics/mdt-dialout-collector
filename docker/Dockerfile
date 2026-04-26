# Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
# Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


ARG VERSION=stable-slim
ARG PLATFORM=linux/amd64

FROM --platform=${PLATFORM} debian:${VERSION}

LABEL Author="Salvatore Cuzzilla (Swisscom AG)"

RUN apt-get update && apt-get -y full-upgrade && \
  apt-get install -y \
    bash \
    git \
    cmake \
    build-essential \
    autoconf \
    libtool \
    pkg-config \
    libjsoncpp-dev \
    librdkafka-dev \
    libconfig++-dev \
    libspdlog-dev \
    libzmq3-dev

WORKDIR /tmp/mdt-dialout-collector
COPY scripts/grpc.sh scripts/
COPY scripts/mdt_dialout_collector.sh scripts/
COPY scripts/mdt_dialout_collector.conf /etc/opt/mdt-dialout-collector/

RUN ./scripts/grpc.sh
RUN rm -rf grpc

RUN ./scripts/mdt_dialout_collector.sh

ENTRYPOINT ["/opt/mdt-dialout-collector/bin/mdt_dialout_collector"]
CMD ["-V"]

