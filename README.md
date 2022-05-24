### MDT aka Model Driven Telemetry

[YANG](https://datatracker.ietf.org/doc/html/rfc6020) is the data modeling language chosen by the Telco industry to represent
network devices' configurations and states.

The data, modeled using YANG, is gathered _(or sent)_ from _(to_) the devices over the network using protocols like
[NETCONF\/RESTCONF](https://datatracker.ietf.org/doc/html/rfc6241) and typically encoded using JSON or XML. The data sent/received
via NETCONF is usually going over SSH (or, more generic, TLS).

Around five years ago, Google srated working on a new RPC framework called [gRPC](https://www.grpc.io) which is now adopted by
all the main Vendors to retrieve/send data to the network devices. The most famous implementation are [gNMI](https://github.com/openconfig/gnmi)
and [gRPC Dial-in\/Dial-out](https://xrdocs.io/telemetry/blogs/2017-01-20-model-driven-telemetry-dial-in-or-dial-out/)

### gRPC vs NETCONF _short version_

- gRPC is generally faster to develop with: it's using [Protocol Buffers](https://developers.google.com/protocol-buffers/) as the
Interface Description Language and an ad-hoc compiler to automagically generate the associated code.

- gRPC is out-of-the-box supporting multiple programming languages and gives you the freedom to choose the one which fits best your skills
independently from the existing implementations (the Protobuff file is pre-defining the specs for both client and server).

- gRPC is efficient and scalable: it's taking advantage of the efficiency coming from [HTTP/2](https://datatracker.ietf.org/doc/html/rfc7540)
and thanks to Protocol Buffers, the transit data is Binary encoded.


