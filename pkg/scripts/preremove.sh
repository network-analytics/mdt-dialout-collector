#!/bin/sh
# preremove — runs before package files are removed.
set -e

if command -v systemctl >/dev/null 2>&1; then
    systemctl stop    mdt-dialout-collector >/dev/null 2>&1 || true
    systemctl disable mdt-dialout-collector >/dev/null 2>&1 || true
fi
