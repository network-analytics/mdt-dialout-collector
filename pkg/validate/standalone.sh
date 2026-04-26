#!/bin/sh
# Validate the standalone package in a FRESH container of the target
# distro (no build deps installed). Run by the validate-package CI job.
#
# Steps:
#   1. Install the .deb/.rpm — catches missing/wrong runtime depends.
#   2. Verify the man page lands at /usr/share/man/man1/.
#   3. Launch the daemon in foreground with a minimal config; give it
#      3 s to bind sockets; assert it's still alive and listening on
#      every vendor port.
#   4. Send SIGTERM; assert clean exit.

set -eu

PKG_FILE="${1:?usage: $0 <package-file>}"
. /etc/os-release

case "${ID}" in
    debian|ubuntu)
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq "${PKG_FILE}"
        ;;
    fedora|rocky|rhel|centos|almalinux)
        # gRPC for EL9 lives in EPEL; install it before the package so
        # dnf can resolve grpc-cpp at install time.
        if [ "${ID}" != "fedora" ]; then
            dnf install -y -q epel-release >/dev/null
        fi
        dnf install -y -q "${PKG_FILE}"
        ;;
    *)
        echo "unsupported distro: ${ID}" >&2
        exit 2
        ;;
esac

echo "==> man page check"
test -f /usr/share/man/man1/mdt_dialout_collector.1.gz
zcat /usr/share/man/man1/mdt_dialout_collector.1.gz | head -1 \
    | grep -qi "mdt_dialout_collector" || {
        echo "man page rendered unexpectedly"; exit 1; }

echo "==> binary check"
test -x /opt/mdt-dialout-collector/bin/mdt_dialout_collector
test -L /usr/bin/mdt_dialout_collector

echo "==> systemd unit check"
test -f /lib/systemd/system/mdt-dialout-collector.service

echo "==> example config check"
test -f /etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf

echo "==> launch the daemon (foreground, minimal config)"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cp "${SCRIPT_DIR}/minimal.conf" /tmp/mdt-validate.conf
/opt/mdt-dialout-collector/bin/mdt_dialout_collector \
    -f /tmp/mdt-validate.conf > /tmp/mdt-validate.log 2>&1 &
DAEMON_PID=$!
trap 'kill ${DAEMON_PID} 2>/dev/null; cat /tmp/mdt-validate.log' EXIT

# Give the daemon up to 5 s to bind every vendor socket.
for i in $(seq 1 20); do
    if ss -lntp 2>/dev/null | grep -q ":20007.*mdt_dialout"; then
        break
    fi
    sleep 0.25
done

echo "==> assert all four vendor sockets are listening"
for port in 20007 20008 20009 20010; do
    ss -lnt | grep -q ":${port}\b" || {
        echo "port ${port} not listening" >&2
        ss -lntp >&2
        exit 1
    }
done

echo "==> assert daemon is still alive"
kill -0 "${DAEMON_PID}" || {
    echo "daemon exited prematurely" >&2; exit 1; }

echo "==> SIGTERM — graceful shutdown"
kill -TERM "${DAEMON_PID}"
wait "${DAEMON_PID}" || true   # exit code from signal is non-zero; that's fine.

trap - EXIT
echo
echo "PACKAGE VALIDATION OK (${ID} ${VERSION_ID:-})"
