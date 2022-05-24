### MDT & Dial-out

[YANG](https://datatracker.ietf.org/doc/html/rfc6020) is the data modeling language chosen by the Telco industry to represent
network devices' configurations and states.

The data is gathered (or sent) from (to) the devices over the network using protocols like [NETCONF](https://localhost) and typically
encoded using JSON or XML. The data sent via NETCONF is usually traveling over SSH (or, more generic,  TLS).

