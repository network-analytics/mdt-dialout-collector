### Build@CentOS 8 Stream (CentOS-Stream-8-x86_64-20221125-boot)

- Install the necessary tools to build
```SHELL
#yum install vim epel-release tmux wget mc most mlocate (optional)
yum install bash git cmake autoconf libtool pkg-config gcc-toolset-11
```

- Build & install the gRPC framework
```SHELL
cd /root
scl enable gcc-toolset-11 bash

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
```

- Build & install the collector deamons (Run the collector natively)
```SHELL
cd /root
scl enable gcc-toolset-11 bash

export PATH="/root/.local/bin:$PATH"
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig/

yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel

cd /tmp
git clone https://github.com/edenhill/librdkafka.git
cd librdkafka
./configure
make
make install

sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

cd /opt
git clone https://github.com/scuzzilla/mdt-dialout-collector.git

cd mdt-dialout-collector
mkdir build
cd build
cmake ../

make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
```

- Build & install the collector libraries (Integrate the collector, via ZMQ, with [pmacct](https://github.com/pmacct/pmacct))
```SHELL
cd /root
scl enable gcc-toolset-11 bash

export PATH="/root/.local/bin:$PATH"
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/root/.local/lib/pkgconfig:/root/.local/lib64/pkgconfig:/usr/local/lib/pkgconfig/
ln -s /root/.local/bin/grpc_cpp_plugin /usr/local/bin/grpc_cpp_plugin

yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel

cd /tmp
git clone https://github.com/edenhill/librdkafka.git
cd librdkafka
./configure
make
make install

sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

cd /opt
git clone https://github.com/scuzzilla/mdt-dialout-collector.git

cd /opt/mdt-dialout-collector
./autogen.sh
CPPFLAGS="-I/root/.local/include" ./configure

make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
make install
```

