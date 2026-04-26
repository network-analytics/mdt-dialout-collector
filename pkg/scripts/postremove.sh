#!/bin/sh
# postremove — runs after package files are removed.
# Only act on a true uninstall ($1 == "remove"/"purge" on deb, "0" on rpm),
# not on an upgrade.
set -e

case "${1:-}" in
    purge|0)
        rm -f /usr/lib/tmpfiles.d/mdt-dialout-collector.conf
        rm -rf /var/run/mdt-dialout-collector
        if getent passwd mdt-dialout-collector >/dev/null 2>&1; then
            if command -v userdel >/dev/null 2>&1; then
                userdel mdt-dialout-collector >/dev/null 2>&1 || true
            fi
        fi
        ;;
esac

if command -v systemctl >/dev/null 2>&1; then
    systemctl daemon-reload >/dev/null 2>&1 || true
fi
