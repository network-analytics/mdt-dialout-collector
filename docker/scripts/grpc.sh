#!/bin/bash


# Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


set -o errexit
set -o nounset

git clone --recurse-submodules -b v1.45.2 --depth 1 --shallow-submodules https://github.com/grpc/grpc

export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"

cd grpc
mkdir -p cmake/build
pushd cmake/build

cmake -DgRPC_INSTALL=ON \
-DCMAKE_BUILD_TYPE=Release \
-DgRPC_BUILD_TESTS=OFF \
-DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
-DABSL_PROPAGATE_CXX_STD=ON \
-DgRPC_ABSL_PROVIDER=module \
-DgRPC_CARES_PROVIDER=module \
-DgRPC_PROTOBUF_PROVIDER=module \
-DgRPC_RE2_PROVIDER=module \
-DgRPC_SSL_PROVIDER=module \
-DgRPC_ZLIB_PROVIDER=module \
../..

make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`

make install
popd

