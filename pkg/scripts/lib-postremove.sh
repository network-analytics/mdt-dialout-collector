#!/bin/sh
# postremove (library variant) — drop the removed library from the loader
# cache. The ld.so.conf.d entry is a package file, removed by the manager.
set -e

if command -v ldconfig >/dev/null 2>&1; then
    ldconfig
fi
