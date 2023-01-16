### Multi-vendor (async) gRPC dial-out collector

Telcos are usually dealing with a considerable amount of devices, typically in a multi-vendor environment.
Of course, all of them are equipped with the *state-of-the-art* telemetry technologies ... Well, **unfortunately**,
we all know that most of the time they're not!

On the other hand "legacy" protocols like *SNMP* are still playing a relevant role and concepts like standardized data modeling
are still not widely adopted.

However, within this conservative context, some technologies are emerging and their adoption by important Service Providers
*(AT&T, Comcast, Verizon, BT, Deutsche Telekom, Swisscom ...)* are making them **a de facto standard** in the world of Network Telemetry.

Recently, I had a closer look to some of these technologies, specifically I've been happily involved with developing an
application based on the gRPC framework to be able to collect data modeled via YANG from a multi-vendor network.

In the next paragraphs, at first, I would like to shortly introduce you to some of these technologies and later on describe the core
features of the [**gRPC dial-out collector**](https://github.com/scuzzilla/mdt-dialout-collector) I've developed.

### Model-Driven Telemetry *(MDT)*

Model-Driven Telemetry's main goal is guiding the Network Operators with choosing the right technologies in order to help them to improve the way
they monitor & automate their Networks. For example, MDT is preferring data **PUSH** over the traditional **PULL** model to improve in terms of *scalability*.
MDT is also helping with making sure that the Network Data is *standardized* across different vendors using modeling languages like YANG.

Essentially without competitors, [**YANG**](https://datatracker.ietf.org/doc/html/rfc6020) is the data modeling language chosen by the
Telcos to describe network devices' configurations and states.

The information, modeled via YANG, is flowing over the network using protocols like [**NETCONF/RESTCONF**](https://datatracker.ietf.org/doc/html/rfc6241)
and is encoded using XML/JSON. The protocol operations are performed as Remote Procedure Calls (RPCs) and the data sent/received by the devices is
carried over SSH/HTTPs.

In 2016 Google released a new RPC framework called [**gRPC**](https://www.grpc.io) which, together with NETCONF/RESTCONF is
adopted by all the major Vendors to send/receive data from the network. Currently the main implementations are
[**gNMI**](https://github.com/openconfig/gnmi) and [**gRPC Dial-in/Dial-out**](https://xrdocs.io/telemetry/blogs/2017-01-20-model-driven-telemetry-dial-in-or-dial-out/)

### gRPC pros *(vs NETCONF/RESTCONF)*

- gRPC is generally faster to develop with: it's using [**Protocol Buffers**](https://developers.google.com/protocol-buffers/) as the
**I**nterface **D**escription **L**anguage and an ad-hoc compiler to *automagically* generate the associated skeleton code.

- gRPC is *out-of-the-box* supporting multiple programming languages and gives you the freedom to choose the one which fits best your skills,
independently from the existing implementations (the Protobuff file is pre-defining the specs for both client and server).

- gRPC is efficient and scalable: it's taking advantage of the efficiency coming from [**HTTP/2**](https://datatracker.ietf.org/doc/html/rfc7540)
and thanks to Protocol Buffers, the exchanged data is binary encoded which considerably reduces the message size.

### gNMI vs gRPC Dial-in/gRPC Dial-out

gNMI (**g**RPC **N**etwork **M**anagement **I**nterface) is using the gRPC framework and a [**Standardized Protobuff**](https://www.openconfig.net/)
file to implement a solution to **fully operate** the network.

With gRPC dial-in/dial-out The data-stream is *(of course)* always PUSH(ed) from the router, however in case of Dial-in the connection
is initiated by the collector, vice-versa with Dial-out the connection is initiated by the router. The biggest benefit of Dial-in over Dial-out
is that you're going to have a single channel usable for both telemetry & routers configuration.

---

### MDT Dial-out collector

After this quick & dirty ramp-up with Network Telemetry technologies, I'd like to focus on the [**mdt-dialout-collector**](https://github.com/scuzzilla/mdt-dialout-collector) main features.
The application is using the  gRPC framework to implement a multi-vendor gRPC Dial-out collector and is supporting
the [**Cisco's gRPC Dial-out .proto file**](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/mdt_grpc_dialout/mdt_grpc_dialout.proto)
, the [**Juniper's gRPC Dial-out .proto file**](https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html)
and the [**Huawei's gRPC Dial-out .proto file**](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files).

The collector functionalities are logically grouped into three categories and each one of them is including multiple options.
The **Data Collection** block is including all configuration parameters associated with the daemons behavior,
while the **Data Manipulation/Enrichment** block is taking care of conveniently transforming the in-transit data-stream up to Kafka.
(Open the image in a new tab to Zoom-In)

![alt text](https://www.alfanetti.org/images/monitoring/mdt_dialout_collector/mdt_dialout_collector_arch.jpg "mdt-dialout-collector, high-level arch")

Depending on the selected Vendor/Operating System, the supported encondings are JSON, GPB-KV and GPB-Comapct (Huawei openconfig-interfaces).

| Vendor | OS Version                     |   Encoding   |      .proto file                                                                                                                                                              |
|--------|--------------------------------|--------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Cisco  | XR  (7.4.1@NSC-540)            | JSON, GPB-KV | [XR Telemetry .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/telemetry.proto)                              |
| Cisco  | XE  (17.06.01prd7@C8000V)      | GPB-KV       | [XE Telemetry .proto](https://github.com/ios-xr/model-driven-telemetry/blob/ebc059d77f813b63bb5a3139f5178ad11665d49f/protos/66x/telemetry.proto)                              |
| Juniper| Junos  (20.4R3-S2.6@mx10003)   | JSON-GNMI    | [Junos Telemetry .proto (Download section)](https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html)  |
| Huawei | VRP (V800R021C10SPC300T@NE40E) | JSON         | [VRP Telemetry .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                               |
| Huawei | VRP (V800R021C10SPC300T@NE40E) | GPB-Compact  | [OpenConfig Interfaces .proto](https://support.huawei.com/enterprise/en/doc/EDOC1100139549/40577baf/common-proto-files)                                                       |

#### Configuration parameters

The default configuration location is following the [FHS](https://refspecs.linuxfoundation.org/fhs.shtml) recommendation, therefore, by default:

- the binary is available at **"/opt/mdt-dialout-collector/bin/mdt_dialout_collector"**
- the configuration should be available at **"/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf"**

However, via command-line, it's possible to specify an alternative location for the configuration.
(the chosen flag is -f).

Here below is a verbosely commented *example* of configuration:
```SHELL
#### mdt-dialout-collector - main

## writer-id:
## default = "mdt-dialout-collector"
writer_id = "mdt-dout-collector-01";

## physical interface where to bind the daemon
iface = "eth0";

## socket dedicated to the cisco's data-stream
ipv4_socket_cisco = "0.0.0.0:10007";

## socket dedicated to the huawei's data-stream
ipv4_socket_huawei = "0.0.0.0:10008";

## socket dedicated to the juniper's data-stream
ipv4_socket_juniper = "0.0.0.0:10009";

## network replies: fine control on the amount of messages received within a single
## session - valid range: "0" < replies < "1000" - (default = "0" => unlimited)
replies_cisco = "10";
replies_juniper = "100";
replies_huawei = "1000";

## workers (threads) per vendor:
## default = "1" | max = "5"
cisco_workers = "1";
juniper_workers = "1";
huawei_workers = "1";

## logging:
## Syslog support:
## default => syslog = "false" | facility (static) default => LOG_USER
syslog = "true";

## Syslog Facility:
## default => syslog_facility = "LOG_USER" | supported [LOG_DAEMON, LOG_USER, LOG_LOCAL(0..7)]
syslog_facility = "LOG_LOCAL3";

## Syslog Ident:
## default => syslog_ident = "mdt-dialout-collector"
syslog_ident = "mdt-dout-collector-01";

## Console support:
## default => console_log = "true"
console_log = "false";

## Severity level:
## default => spdlog_level = "info" | supported [debug, info, warn, error, off]
spdlog_level = "debug";


#### mdt-dialout-collector - data-flow manipulation

## simplified JSON after GPB/GPB-KV decoding:
## default = "true"
enable_cisco_gpbkv2json = "false";

## standard JSON after GPB/GPB-KV deconding:
## default = "false"
enable_cisco_message_to_json_string = "true";

## data-flow enrichment with node_id/platform_id:
## default = "false"
## for additional details refer to "csv/README.md" or "ptm/README.md"

## CSV format:
## default = "false"
enable_label_encode_as_map = "false";
## label_map_csv_path:
## default = "/opt/mdt_dialout_collector/csv/label_map.csv"
label_map_csv_path = "/define/here/your/custom_path.csv"

## PTM format (pmacct's pretag):
## default = "false"
enable_label_encode_as_map_ptm = "true";
## label_map_ptm_path:
## default = "/opt/mdt_dialout_collector/ptm/label_map.ptm"
label_map_ptm_path = "/define/here/your/custom_path.ptm"


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

#### How to build

- It's recommended to compile gRPC and all the associated libraries from scratch.
The [gRPC's Quick start guide](https://grpc.io/docs/languages/cpp/quickstart/) is describing in detail the compile/install procedure. If
you're running a Debian derived Linux distribution you can also refer to the [Alfanetti](https://www.alfanetti.org/grpc-compile-debian.html) documentation.

- [Debian based systems](https://github.com/scuzzilla/mdt-dialout-collector/blob/main/debian_INSTALL.md)

- [CentOS based systems](https://github.com/scuzzilla/mdt-dialout-collector/blob/main/centos_INSTALL.md)

---

#### Licenses matrix

|  Libraries / .proto files                                                                                                                                                   | License                   |
|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|
| [JsonCpp](https://github.com/open-source-parsers/jsoncpp)                                                                                                                   | MIT License               |
| [librdkafka](https://github.com/edenhill/librdkafka)                                                                                                                        | BSD 2-Clause License      |
| [cppzmq](https://github.com/zeromq/cppzmq)                                                                                                                                  | MIT License               |
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
