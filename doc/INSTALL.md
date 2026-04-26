# Installing mdt-dialout-collector

The project is distributed as native packages — `.deb` for Debian /
Ubuntu and `.rpm` for Fedora / Rocky / RHEL — built per release by the
[`release.yml`](../.github/workflows/release.yml) GitHub Actions
workflow.

Two flavors per distro:

| Package | Purpose |
|---|---|
| `mdt-dialout-collector` | Standalone daemon. Ships the binary, a systemd unit, the example config, the man page. |
| `mdt-dialout-collector-lib` | Library variant for [pmacct/pmtelemetryd](https://github.com/pmacct/pmacct) integration. Ships `libgrpc_collector.so` + the C bridge header + a pkg-config file. |

Both packages link against the **distro's own gRPC** (no vendored
runtime), which keeps install size small and lets `apt`/`dnf` handle
gRPC security updates alongside the rest of the system.

## Supported distributions

| Distro | gRPC source | Notes |
|---|---|---|
| Debian 12 (bookworm) | distro repo (gRPC 1.51) | |
| Debian 13 (trixie)   | distro repo (gRPC 1.66) | |
| Ubuntu 24.04 (noble) | distro repo (gRPC 1.51) | |
| Fedora (current)     | distro repo (gRPC 1.62+) | |

**Rocky Linux 9 / RHEL 9** packages are not produced for v1.2.0 — EL9's
stock protobuf (3.14) lacks `has_*()` accessors for `oneof` members
(added in protobuf 3.15) which the Cisco telemetry path uses. EL9 builds
from source still work via [INSTALL-FROM-SOURCE.md](INSTALL-FROM-SOURCE.md);
binary packages are tracked for v1.3.

Earlier Ubuntu LTSes (22.04 and older) ship a gRPC version below the
project's minimum (1.45) and are intentionally not packaged for.

## Install (Debian / Ubuntu)

Grab the matching `.deb` from the latest [GitHub
Release](https://github.com/network-analytics/mdt-dialout-collector/releases),
then:

```sh
# Standalone daemon
sudo apt install ./mdt-dialout-collector_<version>_<distro>_amd64.deb

# Library for pmacct/pmtelemetryd integration
sudo apt install ./mdt-dialout-collector-lib_<version>_<distro>_amd64.deb
```

`apt` resolves the gRPC + protobuf + libssl + libconfig++ + librdkafka +
libjsoncpp + libzmq + libspdlog runtime deps automatically.

## Install (Fedora)

```sh
# Standalone daemon
sudo dnf install ./mdt-dialout-collector-<version>-1.<distro>.x86_64.rpm

# Library variant
sudo dnf install ./mdt-dialout-collector-lib-<version>-1.<distro>.x86_64.rpm
```

## After install

Edit the configuration:

```sh
sudo $EDITOR /etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf
```

Start and enable the daemon:

```sh
sudo systemctl enable --now mdt-dialout-collector
sudo systemctl status mdt-dialout-collector
```

Logs land in journald (or syslog/console depending on the `syslog` /
`console_log` config keys):

```sh
journalctl -u mdt-dialout-collector -f
```

## Reference docs

- `man mdt_dialout_collector` — quick reference on the command line.
- `/usr/share/doc/mdt-dialout-collector/CONFIG-KEYS` — every config key
  with description and default.
- [doc/integration-with-pmtelemetryd.md](integration-with-pmtelemetryd.md)
  — how to wire the library variant into pmacct.
- [doc/network-devices-conf-snip.md](network-devices-conf-snip.md) —
  per-vendor router configuration snippets.

## Building from source

If you need a build for a distro that's not in the matrix above, see
[INSTALL-FROM-SOURCE.md](INSTALL-FROM-SOURCE.md). The legacy
[`install.sh`](../install.sh) script remains in the tree as a developer
convenience but is no longer the recommended install path.
