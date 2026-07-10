#!/bin/sh
# postinstall (library variant) — refresh the loader cache so consumers
# (pmtelemetryd) resolve libgrpc_collector.so.0 from /opt at runtime.
# Idempotent: safe to re-run on package upgrade.
set -e

if command -v ldconfig >/dev/null 2>&1; then
    ldconfig
fi
