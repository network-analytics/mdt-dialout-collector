###  Build@Debian Stable (debian-11.5.0-amd64-netinst)

- Install the necessary tools to build
```SHELL
#apt install sudo tmux vim-nox wget mc most locate (optional)
apt install bash git cmake build-essential autoconf libtool pkg-config
```

- Build & install the gRPC framework
```SHELL
cd /root/
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

- Build & install the collector deamons
```SHELL
export PATH="/root/.local/bin:$PATH"

apt install -y libjsoncpp-dev librdkafka-dev libconfig++-dev libspdlog-dev libzmq3-dev libssl-dev

cd /opt
git clone https://github.com/scuzzilla/mdt-dialout-collector.git

cd mdt-dialout-collector
mkdir build
cd build
cmake ../

make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
```

- Build & install the collector libraries
```SHELL
export PATH="/root/.local/bin:$PATH"
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/root/.local/lib/pkgconfig
ln -s /root/.local/bin/grpc_cpp_plugin /usr/local/bin/grpc_cpp_plugin

apt install -y libjsoncpp-dev librdkafka-dev libconfig++-dev libspdlog-dev libzmq3-dev libssl-dev

cd /opt
git clone https://github.com/scuzzilla/mdt-dialout-collector.git

cd mdt-dialout-collector
./autogen.sh
CPPFLAGS="-I/root/.local/include -I/usr/include/jsoncpp" ./configure

make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
make install
```

