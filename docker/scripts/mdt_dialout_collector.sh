#!/bin/bash


# Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
# Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


set -o errexit
set -o nounset

cd /opt
git clone https://github.com/network-analytics/mdt-dialout-collector.git

export PATH="/root/.local/bin:$PATH"
cd mdt-dialout-collector;
mkdir build;
cd build;
cmake ../;
make -j

