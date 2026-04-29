[![Build status](https://github.com/network-analytics/mdt-dialout-collector/actions/workflows/ci.yaml/badge.svg?branch=main)](https://github.com/network-analytics/mdt-dialout-collector/actions/workflows/ci.yaml) [![Coverage](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/network-analytics/mdt-dialout-collector/badges/coverage.json)](https://github.com/network-analytics/mdt-dialout-collector/actions/workflows/coverage.yml) [![Release](https://img.shields.io/github/v/release/network-analytics/mdt-dialout-collector?sort=semver)](https://github.com/network-analytics/mdt-dialout-collector/releases)

## Table of Content

<!--ts-->
   * [Introduction](#introduction)
   * [Deployment options](#deployment-options)
      * [Standalone binary with mdt-dialout-collector](#standalone-binary-with-mdt-dialout-collector)
      * [Library/Header integration with pmtelemetryd](#libraryheader-integration-with-pmtelemetryd)
   * [Build/Install](#buildinstall)
   * [References](#references)
<!--te-->

## Introduction
**mdt-dialout-collector** & **gRPC dial-out libraries** are leveraging the [**gRPC Framework**](https://grpc.io/) to implement a multi-vendor gRPC Dial-out collector.
The [doc/Changelog](https://github.com/network-analytics/mdt-dialout-collector/blob/main/doc/Changelog) file is including additional details about the supported network devices.

The collector functionalities can be logically grouped into three categories:

1. **Data Collection**   - they are steering the daemon(s) behavior.
2. **Data Manipulation** - they are conveniently transforming the in-transit data-stream.
3. **Data Delivery**     - they are inter-connecting the collector with the next stage in the pipeline.

The [doc/CONFIG-KEYS](https://github.com/network-analytics/mdt-dialout-collector/blob/main/doc/CONFIG-KEYS) file is including the description for each one of the available options.

Additional capabilities:

- **IPv4 and IPv6** listening sockets (`socket_<vendor>` keys accept both `0.0.0.0:port` and `[::]:port` / `[2001:db8::1]:port`). The legacy `ipv4_socket_<vendor>` keys remain honored as deprecated aliases.
- **Server-side TLS** on every vendor's gRPC port (`enable_tls`, `tls_cert_path`, `tls_key_path`). Off by default — existing configs keep working unchanged.
- **Graceful shutdown** in library mode via the `stop_grpc_dialout_collector(void)` C symbol exported by `libgrpc_collector` — drains in-flight RPCs and joins all worker threads.
- **End-to-end test harness** under [tests/e2e/](https://github.com/network-analytics/mdt-dialout-collector/tree/main/tests/e2e) (synthetic per-vendor gRPC clients + redpanda + the collector under podman). `bash tests/e2e/run.sh` runs the standalone path; `--pmtelemetryd` runs the library path; `--tls` runs the TLS path; `--ipv6` runs against an IPv6 listening socket; `--next` runs against the latest gRPC/Protobuf stack.

## Deployment options

The gRPC dial-out data-collection functionality can be deployed in two ways:

### Standalone binary with mdt-dialout-collector
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
Additionally, the default configuration file can be further specified via the following command line:
```TEXT
/opt/mdt-dialout-collector/bin/mdt_dialout_collector -f <file.conf>
```

### Library/Header integration with pmtelemetryd
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
there is one main pmtelemetryd [CONFIG-KEYS](https://github.com/pmacct/pmacct/blob/master/CONFIG-KEYS) which is mandatory in order to enable the embedded gRPC dial-out collector:
```TEXT
KEY:     telemetry_daemon_grpc_collector_conf
DESC:    Points to a file containing the configuration of the gRPC collector thread. An
         example of the configuration plus all available config keys is available here:
         https://github.com/network-analytics/mdt-dialout-collector
DEFAULT: none
```

## Install

Native packages (`.deb` for Debian / Ubuntu, `.rpm` for Fedora) are built per release by the [release.yml](https://github.com/network-analytics/mdt-dialout-collector/blob/main/.github/workflows/release.yml) GitHub Actions workflow and attached to each [GitHub Release](https://github.com/network-analytics/mdt-dialout-collector/releases). Two flavors per distro:

- `mdt-dialout-collector` — standalone daemon (binary, systemd unit, example config, man page).
- `mdt-dialout-collector-lib` — library variant for [pmacct/pmtelemetryd](https://github.com/pmacct/pmacct) integration (`libgrpc_collector.so` + C bridge header + pkg-config file).

Both link against the distro's own gRPC; `apt`/`dnf` resolves the rest of the runtime deps automatically.

```SHELL
# Debian / Ubuntu
sudo apt install ./mdt-dialout-collector_<version>_<distro>_amd64.deb

# Fedora
sudo dnf install ./mdt-dialout-collector-<version>-1.fc<release>.x86_64.rpm

sudo $EDITOR /etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf
sudo systemctl enable --now mdt-dialout-collector
```

Prefer a container? Pull the latest image from Docker Hub:

```SHELL
docker pull scuzzilla/mdt-dialout-collector:latest        # standalone daemon
docker pull scuzzilla/mdt-dialout-collector-lib:latest    # library variant (FROM-base for pmtelemetryd builds)
```

Full install matrix (including the container run/mount recipe) and post-install steps: [doc/INSTALL.md](doc/INSTALL.md). On-line reference: `man mdt_dialout_collector` after install.

### Building from source

For developers and for distros not in the release matrix, see [doc/INSTALL-FROM-SOURCE.md](doc/INSTALL-FROM-SOURCE.md). The legacy [`install.sh`](install.sh) is preserved as a developer convenience but is no longer the recommended install path.

## References

- [Integration with PMACCT/pmtelemetryd](https://github.com/network-analytics/mdt-dialout-collector/blob/main/doc/integration-with-pmtelemetryd.md)
- [Network devices configuration snippets](https://github.com/network-analytics/mdt-dialout-collector/blob/main/doc/network-devices-conf-snip.md)
- [Multivendor (async) gRPC dial-out collector - APNIC Blog](https://blog.apnic.net/2022/10/17/multivendor-async-grpc-dial-out-collector/)
