## Compile/Install gRPC dial-out library/Header for pmtelemetryd

```SHELL
$ sudo /bin/sh -c "$(curl -fsSL https://github.com/network-analytics/mdt-dialout-collector/raw/main/install.sh)" -- -l
```

#### *(sh install.sh -l)* explained

- The gRPC framework source code is cloned by default under the "/root" folder:
```SHELL
# from install.sh
readonly grpc_clone_dir="$HOME/grpc"
```
- The gRPC framework is compiled and installed under the "/root/.local/{bin,include,lib,share}" folders:
```SHELL
# from install.sh
readonly grpc_install_dir="$HOME/.local"
```
- The gRPC dial-out source code is cloned/compiled under the "/opt/mdt-dialout-collector" folder:
```SHELL
# from install.sh
readonly mdt_install_dir="/opt/mdt-dialout-collector"
```
- The building process is generating both the library and the header file required to build pmtelemetryd with gRPC dial-out support:
```SHELL
/usr/local/lib/libgrpc_collector.la
/usr/local/include/grpc_collector_bridge/grpc_collector_bridge.h
```

## Compile/Install pmtelemetryd with gRPC dial-out support enabled

```SHELL
$ sudo apt install libzmq3-dev libjansson-dev librdkafka-dev

$ cd /opt
$ sudo git clone https://github.com/pmacct/pmacct.git
$ cd /opt/pmacct
$ sudo ./autogen.sh
$ sudo ./configure --enable-debug --enable-zmq --enable-jansson --enable-kafka --enable-grpc-collector
$ sudo make -j
$ sudo make install
```
