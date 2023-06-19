## Compile/Install gRPC dial-out library/Header for pmtelemetryd

```SHELL
$ sudo /bin/sh -c "$(curl -fsSL https://github.com/network-analytics/mdt-dialout-collector/raw/main/install.sh)" -- -l
```

### sh install.sh -l main actions

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
