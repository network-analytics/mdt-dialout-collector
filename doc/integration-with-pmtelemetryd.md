## Table of Content

<!--ts-->
   * [Introduction](#introduction)
   * [Testing Environment](#testing-environment)
   * [Compile/Install gRPC dial-out library/Header for pmtelemetryd](#compileinstall-gRPC-dial-out-libraryheader-for-pmtelemetryd)
   * [Compile/Install pmtelemetryd with gRPC dial-out support enabled](#compileinstall-pmtelemetryd-with-gRPC-dial-out-support-enabled)
   * [pmtelemetryd's minimal configuration snippet](#pmtelemetryds-minimal-configuration-snippet)
   * [gRPC's dial-out minimal configuration snippet](#gRPCs-dial-out-minimal-configuration-snippet)
<!--te-->

## Introduction

The following paragraphs outline the steps necessary to integrate the gRPC dial-out data collection functionality into pmacct/pmtelemetryd.
I've included a minimal set of configuration snippets that can serve to verify the installation as well as act as a starting point for more intricate scenarios.

## Testing Environment

```SHELL
$ sudo cat /etc/os-release

PRETTY_NAME="Debian GNU/Linux 11 (bullseye)"
NAME="Debian GNU/Linux"
VERSION_ID="11"
VERSION="11 (bullseye)"
VERSION_CODENAME=bullseye
ID=debian
HOME_URL="https://www.debian.org/"
SUPPORT_URL="https://www.debian.org/support"
BUG_REPORT_URL="https://bugs.debian.org/"
```

## Install the gRPC dial-out library/Header for pmtelemetryd

The recommended path is the `mdt-dialout-collector-lib` package (see [doc/INSTALL.md](INSTALL.md) for the full matrix):

```SHELL
# Debian / Ubuntu
sudo apt install ./mdt-dialout-collector-lib_<version>_<distro>_amd64.deb

# Rocky / RHEL (gRPC lives in EPEL)
sudo dnf install epel-release
sudo dnf install ./mdt-dialout-collector-lib-<version>-1.<distro>.x86_64.rpm

# Fedora
sudo dnf install ./mdt-dialout-collector-lib-<version>-1.fc<release>.x86_64.rpm
```

The library variant ships:

```SHELL
/opt/mdt-dialout-collector/lib/libgrpc_collector.so
/opt/mdt-dialout-collector/lib/pkgconfig/grpc_collector.pc
/opt/mdt-dialout-collector/include/grpc_collector_bridge/grpc_collector_bridge.h
```

For developers wanting to build the library from source, see [doc/INSTALL-FROM-SOURCE.md](INSTALL-FROM-SOURCE.md). The legacy `install.sh -l` flow is preserved but no longer the recommended path.

## Compile/Install pmtelemetryd with gRPC dial-out support enabled

```SHELL
sudo apt install libzmq3-dev libjansson-dev librdkafka-dev

cd /opt
sudo git clone https://github.com/pmacct/pmacct.git
cd /opt/pmacct
sudo ./autogen.sh
sudo PKG_CONFIG_PATH="/opt/mdt-dialout-collector/lib/pkgconfig:${PKG_CONFIG_PATH:-}" \
    ./configure --enable-debug --enable-zmq --enable-jansson --enable-kafka --enable-grpc-collector
sudo make -j
sudo make install
```

## pmtelemetryd's minimal configuration snippet

```SHELL
$ sudo cat /root/etc/pmtelemetryd.conf

! ### Generic Settings
core_proc_name: pmtelemetryd-grpc
pidfile: /root/var/run/pmtelemetryd-grpc
logfile: /root/var/log/pmacct/pmtelemetryd.log
!
! ### gRPC dial-out Settings
telemetry_daemon_decoder: json
telemetry_daemon_grpc_collector_conf: /root/etc/pmtelemetryd-grpc-dialout.conf
!
! ### Kafka Settings
telemetry_daemon_msglog_output: json
telemetry_daemon_msglog_kafka_topic: kafka.topic
telemetry_daemon_msglog_kafka_config_file: /root/etc/kafka.conf


$ sudo cat /root/etc/kafka.conf

global, compression.type, snappy
global, queue.buffering.max.messages, 10000000
global, batch.size, 2147483647
global, batch.num.messages, 1000000
global, linger.ms, 200
global, client.id, debian
global, security.protocol, plaintext
global, metadata.broker.list, 192.168.100.1:9092
```

## gRPC's dial-out minimal configuration snippet

```SHELL
$ cat /root/etc/pmtelemetryd-grpc-dialout.conf

iface = "enp1s0";
socket_cisco = "192.168.100.254:10001";
data_delivery_method = "zmq";

spdlog_level = "debug";

enable_cisco_gpbkv2json = "false";
enable_cisco_message_to_json_string = "true";
```
