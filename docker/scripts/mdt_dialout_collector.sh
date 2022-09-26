#!/bin/bash


# Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


set -o errexit
set -o nounset

cd /opt
git clone https://github.com/scuzzilla/mdt-dialout-collector.git

export PATH="/root/.local/bin:$PATH"
cd mdt-dialout-collector;
mkdir build;
cd build;
cmake ../;
make -j

