#!/bin/sh
# postinstall — runs after the package files are unpacked.
# Idempotent: safe to re-run on package upgrade.
set -e

# Service account for the daemon.
if ! getent passwd mdt-dialout-collector >/dev/null 2>&1; then
    if command -v useradd >/dev/null 2>&1; then
        useradd --system --no-create-home --shell /usr/sbin/nologin \
            --comment "mdt-dialout-collector daemon" \
            mdt-dialout-collector
    elif command -v adduser >/dev/null 2>&1; then
        adduser --system --no-create-home --shell /usr/sbin/nologin \
            --group mdt-dialout-collector
    fi
fi

# PID-file directory referenced by core_pid_folder in the example config.
mkdir -p /var/run/mdt-dialout-collector
chown mdt-dialout-collector:mdt-dialout-collector /var/run/mdt-dialout-collector

# tmpfiles.d — required so /var/run/mdt-dialout-collector survives a reboot
# (because /var/run is a tmpfs on most modern distros).
cat >/usr/lib/tmpfiles.d/mdt-dialout-collector.conf <<EOF
d /var/run/mdt-dialout-collector 0755 mdt-dialout-collector mdt-dialout-collector -
EOF

if command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload || true
fi

echo
echo "mdt-dialout-collector installed."
echo "Edit /etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf, then:"
echo "    systemctl enable --now mdt-dialout-collector"
echo
