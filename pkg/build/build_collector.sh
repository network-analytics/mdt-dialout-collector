#!/bin/sh
# Build the standalone binary AND the library variant in one cmake pass.
# Outputs into:
#   bin/mdt_dialout_collector       — for nfpm.standalone.yaml
#   build-pkg/lib/libgrpc_collector.so — for nfpm.library.yaml
#   build-pkg/grpc_collector.pc     — for nfpm.library.yaml

set -eu

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "${REPO_ROOT}"

# Standalone — uses the existing CMakeLists.txt path that produces
# bin/mdt_dialout_collector linked against system gRPC.
cmake -S . -B build-pkg \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/mdt-dialout-collector \
    -DCMAKE_INSTALL_RPATH=/opt/mdt-dialout-collector/lib
cmake --build build-pkg -j"$(nproc)"

# Library — autotools path produces libgrpc_collector.so via libtool.
# Reuses the same compiled objects via the shared src/ tree.
./autogen.sh >/dev/null
./configure --prefix=/opt/mdt-dialout-collector --quiet >/dev/null
make -j"$(nproc)" >/dev/null
mkdir -p build-pkg/lib
# libtool-staged .so lives under src/.libs/
cp -L src/.libs/libgrpc_collector.so build-pkg/lib/libgrpc_collector.so

# Generate the .pc file from the .pc.in template.
VERSION_STR="$(cat VERSION 2>/dev/null || echo 0.0.0)"
sed -e "s|@prefix@|/opt/mdt-dialout-collector|g" \
    -e "s|@exec_prefix@|/opt/mdt-dialout-collector|g" \
    -e "s|@libdir@|/opt/mdt-dialout-collector/lib|g" \
    -e "s|@includedir@|/opt/mdt-dialout-collector/include|g" \
    -e "s|@PACKAGE_VERSION@|${VERSION_STR}|g" \
    grpc-collector.pc.in > build-pkg/grpc_collector.pc

# Strip symbols (smaller package payload).
strip --strip-unneeded bin/mdt_dialout_collector       || true
strip --strip-unneeded build-pkg/lib/libgrpc_collector.so || true

# Compress the man page (Debian/Fedora policy: gzipped under section dir).
mkdir -p build-pkg/man
gzip -9 -n -c pkg/man/mdt_dialout_collector.1 > build-pkg/man/mdt_dialout_collector.1.gz

echo "build OK:"
ls -lh bin/mdt_dialout_collector build-pkg/lib/libgrpc_collector.so \
       build-pkg/man/mdt_dialout_collector.1.gz
