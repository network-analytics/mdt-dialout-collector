### MDT & Dial-out

[YANG](https://datatracker.ietf.org/doc/html/rfc6020) is the data modeling language chosen by the Telco industry to represent
network devices' configurations and states.

The data, modeled using YANG, is gathered _(or sent)_ from _(to_) the devices over the network using protocols like [NETCONF\/RESTCONF]
(https://datatracker.ietf.org/doc/html/rfc6241) and typically encoded using JSON or XML. The data sent/received via NETCONF is usually
going over SSH (or, more generic, TLS).


