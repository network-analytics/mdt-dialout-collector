# mdt-dialout-collector

[![Build status](https://github.com/scuzzilla/mdt-dialout-collector/workflows/ci/badge.svg?branch=main)](https://github.com/scuzzilla/mdt-dialout-collector/actions)

**mdt-dialout-collector** is leveraging the [**gRPC Framework**](https://grpc.io/) to implement a multi-vendor gRPC Dial-out collector.
The [**Changelog**](doc/Changelog) file is including additional details about the supported vendors.

The collector functionalities can be logically grouped into three categories:

1. **Data Collection**   - they are steering the daemon(s) behavior.
2. **Data Manipulation** - they are conveniently transforming the in-transit data-stream.
3. **Data Delivery**     - they are inter-connecting the collector with the next stage in the pipeline.

```TEXT
              +--------------------------------------------------+
+---------+   | +------------+   +--------------+   +----------+ |   +------------+
| network |-->| | collection |-->| manipulation |-->| delivery | |-->| pipeline   |
+---------+   | +------------+   +--------------+   +----------+ |   | next stage |
              |              [mdt-dialout-collector]             |   +------------+
              +--------------------------------------------------+
```

The [**CONFIG-KEYS**](doc/CONFIG-KEYS) file is including the description for each one of the available options.

## Build

