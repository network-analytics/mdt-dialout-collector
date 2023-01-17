# mdt-dialout-collector

[![Build status](https://github.com/scuzzilla/mdt-dialout-collector/workflows/ci/badge.svg?branch=main)](https://github.com/scuzzilla/mdt-dialout-collector/actions)

**mdt-dialout-collector** is leveraging the [**gRPC Framework**](https://grpc.io/) to implement a multi-vendor gRPC Dial-out collector.
The [doc/Changelog](https://github.com/scuzzilla/mdt-dialout-collector/blob/reorgdoc/doc/Changelog) file is including additional details about the supported vendors.

The collector functionalities can be logically grouped into three categories:

1. **Data Collection**   - they are steering the daemon(s) behavior.
2. **Data Manipulation** - they are conveniently transforming the in-transit data-stream.
3. **Data Delivery**     - they are inter-connecting the collector with the next stage in the pipeline.

The [doc/CONFIG-KEYS](https://github.com/scuzzilla/mdt-dialout-collector/blob/reorgdoc/doc/CONFIG-KEYS) file is including the description for each one of the available options.

## Deployment

mdt-dialout-collector can be deployed in two ways:

### Standalone binary
```TEXT
              +------------------------------------------------------+
+---------+   | +------------+   +--------------+   +--------------+ |   +---------+
| network |-->| | collection |-->| manipulation |-->| kafka client | |-->| kafka   |
+---------+   | +------------+   +--------------+   +--------------+ |   | cluster |
              |              [mdt-dialout-collector]                 |   +---------+
              +------------------------------------------------------+
```
the building process is generating a single binary:
```TEXT
/opt/mdt-dialout-collector/bin/mdt_dialout_collector
```
which, by default, is reading the running options from:
```TEXT
/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf
```
Eventually, The default configuration file can be replaced via command line:
```TEXT
/opt/mdt-dialout-collector/bin/mdt_dialout_collector -f <file.conf>
```

### Embedded into pmtelemetryd
```TEXT
              +---------------------------------------------------------+
+---------+   | +------------+   +--------------+   +-----------------+ |   +------------+
| network |-->| | collection |-->| manipulation |-->| ZMQ (PUSH/PULL) | |-->| pipeline   |
+---------+   | +------------+   +--------------+   +-----------------+ |   | next stage |
              |                    [pmtelemetryd]                       |   +------------+
              +---------------------------------------------------------+
```
the building process is generating both the library and the header file required to build [pmtelemetryd](https://github.com/pmacct/pmacct/blob/master/INSTALL) with gRPC dial-out support:
```
/usr/local/lib/libgrpc_collector.la

/usr/local/include/grpc_collector_bridge/grpc_collector_bridge.h
```
there are two main pmtelemetryd [CONFIG-KEYS](https://github.com/pmacct/pmacct/blob/master/CONFIG-KEYS) which are mandatory in order to enable the embedded gRPC dial-out collector:
```TEXT
KEY:     telemetry_daemon_grpc_collector_socket
DESC:    Enables Streaming Telemetry data collection via gRPC, mutual exclusive with other
         telemetry collection methods. A value like "ipc:///tmp/grpc.sock" is expected, it
         defines the internal ZMQ socket between the gRPC collector thread and pmtelemetryd
         thread. Read more, including instructions on how to build the dependency stack,
         here: https://github.com/scuzzilla/mdt-dialout-collector . This feature requires
         to configure pmacct for compiling with --enable-zmq and --enable-grpc-collector flags.
DEFAULT: none

KEY:     telemetry_daemon_grpc_collector_conf
DESC:    Points to a file containing the configuration of the gRPC collector thread. An
         example of the configuration plus all available config keys is available here:
         https://github.com/scuzzilla/mdt-dialout-collector
DEFAULT: none
```

## Build/Install

[install.sh](https://github.com/scuzzilla/mdt-dialout-collector/blob/reorgdoc/install.sh) is automating the build/install process, taking care of all [dependencies](https://github.com/scuzzilla/mdt-dialout-collector/blob/reorgdoc/doc/Dependencies).

- The Standalone binary can be deployed using:
```SHELL
$ sudo install.sh -b
```

- The Library/Header can be deployed using:
```SHELL
$ sudo install.sh -l
```

## References

-  https://blog.apnic.net/2022/10/17/multivendor-async-grpc-dial-out-collector/
