### MDT aka Model Driven Telemetry

[YANG](https://datatracker.ietf.org/doc/html/rfc6020) is the data modeling language chosen by the Telco industry to represent
network devices' configurations and states.

The data, modeled using YANG, is gathered _(or sent)_ from _(to_) the devices over the network using protocols like
[NETCONF\/RESTCONF](https://datatracker.ietf.org/doc/html/rfc6241) and typically encoded using JSON or XML. The data sent/received
via NETCONF is usually going over SSH (or, more generic, TLS).

In 2016 Google released a new RPC framework called [gRPC](https://www.grpc.io) which is now adopted by
all the main Vendors to retrieve/send data to the network. Currently the main implementations are [gNMI](https://github.com/openconfig/gnmi)
and [gRPC Dial-in\/Dial-out](https://xrdocs.io/telemetry/blogs/2017-01-20-model-driven-telemetry-dial-in-or-dial-out/)

### gRPC vs NETCONF _short version_

- gRPC is generally faster to develop with: it's using [Protocol Buffers](https://developers.google.com/protocol-buffers/) as the
Interface Description Language and an ad-hoc compiler to automagically generate the associated skeleton code.

- gRPC is out-of-the-box supporting multiple programming languages and gives you the freedom to choose the one which fits best your skills,
independently from the existing implementations (the Protobuff file is pre-defining the specs for both client and server).

- gRPC is efficient and scalable: it's taking advantage of the efficiency coming from [HTTP/2](https://datatracker.ietf.org/doc/html/rfc7540)
and thanks to Protocol Buffers, the exchanged data is binary encoded which considerably reduces the message size.

### gRPC Dial-in vs gRPC Dial-out vs gNMI _short version_

gNMI (gRPC Network Management Interface) is using the gRPC framework and a [standardized Protobuff](https://www.openconfig.net/projects/rpc/)
file to implement a solution to fully operate the network.

With what concerning gRPC dial-in/dial-out The data-stream is always pushed-out from the router, however in case of Dial-in the connection
is initiated by the collector, vice-versa with Dial-out the connection is initiated by the router.
The biggest benefit of Dial-in over Dial-out is that you're gonna have a single channel usable for both telemetry & routers configuration.

---

### MDT Dial-out collector

The main aim of this project it to use the gRPC framework to implement a multi-vendor gRPC Dial-out server. Currently the application is implementing
the [Cisco's gRPC Dial-out .proto file](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/mdt_grpc_dialout/mdt_grpc_dialout.proto)
, the [Juniper's gRPC Dial-out .proto file](https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html)
and the [Huawei's gRPC Dial-out .proto file](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files).

With some restrictions, due to the technical limitations imposed by the Vendors' implementations, the supported encodings are JSON, GPB-KV and GPB-Comapct (Huawei openconfig-interfaces only).

| Vendor | OS Version                     |   Encoding   |      .proto file                                                                                                                                                              |
|--------|--------------------------------|--------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Cisco  | XR  (7.4.1@NSC-540)            | JSON, GPB-KV | [XR Telemetry .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/telemetry.proto)                              |
| Cisco  | XE  (17.06.01prd7@C8000V)      | GPB-KV       | [XE Telemetry .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/telemetry.proto)                              |
| Juniper| Junos  (20.4R3-S2.6@mx10003)   | JSON-GNMI    | [Junos Telemetry .proto (Download section)](https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html)  |
| Huawei | VRP (V800R021C10SPC300T@NE40E) | JSON         | [VRP Telemetry .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                               |
| Huawei | VRP (V800R021C10SPC300T@NE40E) | GPB-Compact  | [OpenConfig Interfaces .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                       |

#### How to build

- It's recommended to compile gRPC and all the associated libraries from scratch.
The [gRPC's Quick start guide](https://grpc.io/docs/languages/cpp/quickstart/) is describing in detail the compile/install procedure. If
you're running a Debian derived Linux distribution you can also refer to the [Alfanetti](https://www.alfanetti.org/grpc-compile-debian.html) documentation.

- Libraries like jsoncpp & librdkafka are also required.
```SHELL
Debian$ sudo apt install libjsoncpp-dev librdkafka-dev libconfig++-dev libspdlog-dev
Redhat$ sudo yum install jsoncpp-devel librdkafka-devel libconfig-devel spdlog-devel
```

#### git Clone, Compile and Run
```SHELL
$ cd /opt
$ git clone https://github.com/scuzzilla/mdt-dialout-collector.git

$ cd mdt-dialout-collector
$ mkdir build
$ cmake ../
$ make

$ ./bin/mdt-dialout-collector
```

#### Additional Install notes

If you refer to the [Alfanetti](https://www.alfanetti.org/grpc-compile-debian.html) documentation to compile gRPC, you might want to configure the $PATH variable to include your local
"libs/header" folder.
```SHELL
$ export MY_INSTALL_DIR=$HOME/.local
$ export PATH="$MY_INSTALL_DIR/bin:$PATH"
```

On CentOS you might need to modify the pkg-config path to allow cmake to find all required libraries.
```SHELL
$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig/
```

On CentOS the "spdlog" library should use the external 'fmt' library.
```TEXT
uncomment #define SPDLOG_FMT_EXTERNAL from /usr/include/spdlog/tweakme.h before
running the make command
```

#### Configuration parameters

The default configuration location is following the [FHS](https://refspecs.linuxfoundation.org/fhs.shtml) recommendation, therefore:
- the application itself is located within the "/opt/mdt-dialout-collector/bin" folder
- its configuration is located within the "/etc/opt/mdt-dialout-collector" folder

Here below is an *example* of commented configuration:
```SHELL
#### mdt-dialout-collector - main

## physical interface where to bind the daemon
iface = "eth0";

## socket dedicated to the cisco's data-stream
ipv4_socket_cisco = "0.0.0.0:10007";

## socket dedicated to the huawei's data-stream
ipv4_socket_huawei = "0.0.0.0:10008";

## socket dedicated to the juniper's data-stream
ipv4_socket_juniper = "0.0.0.0:10009";

## network replies: fine control on the amount of messages received within a single
## session - valid range: "10" < replies < "1000" - default = "100"
replies_cisco = "10";
replies_juniper = "100";
replies_huawei = "1000";

## workers (threads) per vendor - default = "1"
cisco_workers = "1";
juniper_workers = "1";
huawei_workers = "1";

## logging:
## Syslog support: default => syslog = "flase" | facility (static) default => LOG_USER
syslog = "true";

## Console support: default => console_log = "true"
console_log = "false";

## Severity level: default => spdlog_level = "info" | supported [debug, info, warn, error, off]
spdlog_level = "debug";


#### mdt-dialout-collector - data-flow manipulation

## simplified JSON after GPB/GPB-KV decoding - default = "true"
enable_cisco_gpbkv2json = "false";

## standard JSON after GPB/GPB-KV deconding - default = "false"
enable_cisco_message_to_json_string = "true";

## data-flow enrichment with node_id/platform_id - default = "false"
## for additional details refer to "csv/README.md"
enable_label_encode_as_map = "true";
## label_map_csv_path - default = "/opt/mdt_dialout_collector/csv/label_map.csv"
label_map_csv_path = "/define/here/your/custom_path.csv"


#### mdt-dialout-collector - kafka-producer
# https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md

bootstrap_servers = "kafka.brockers.net:9093";
topic = "json.topic";
enable_idempotence = "true";
client_id = "mdt-dialout-collector";
# valid options are either plaintext or ssl
security_protocol = "ssl";
ssl_key_location = "/opt/mdt-dialout-collector/cert/collectors.key";
ssl_certificate_location = "/opt/mdt-dialout-collector/cert/collectors.crt";
ssl_ca_location = "/opt/mdt-dialout-collector/cert/sbd_root_ca.crt";
log_level = "0";
```

---

#### Licenses matrix

|  Libraries / .proto files                                                                                                                                                   | License                   |
|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|
| [JsonCpp](https://github.com/open-source-parsers/jsoncpp)                                                                                                                   | MIT License               |
| [librdkafka](https://github.com/edenhill/librdkafka)                                                                                                                        | BSD 2-Clause License      |
| [Modern C++ Kafka API](https://github.com/morganstanley/modern-cpp-kafka)                                                                                                   | Apache License Version 2.0|
| [libconfig](http://hyperrealm.github.io/libconfig/)                                                                                                                         | LGPL v2.1                 |
| [rapidcsv](https://github.com/d99kris/rapidcsv)                                                                                                                             | BSD-3-Clause license      |
| [spdlog](https://github.com/gabime/spdlog)                                                                                                                                  | MIT License               |
| [gRPC](https://github.com/grpc/grpc)                                                                                                                                        | BSD 3-Clause License      |
| [Cisco dial-out .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/mdt_grpc_dialout/mdt_grpc_dialout.proto)  | Apache License Version 2.0|
| [Cisco telemetry .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/telemetry.proto)                         | Apache License Version 2.0|
| [Junos Telemetry .proto (Download section)](https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html)| Apache License Version 2.0|
| [Huawei dial-out .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                           | N/A                       |
| [Huawei telemetry .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                          | N/A                       |
| [mdt-dialout-collector](https://github.com/scuzzilla/mdt-dialout-collector)                                                                                                 | MIT License               |

