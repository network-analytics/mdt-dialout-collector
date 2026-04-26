#!/bin/sh
# Run nfpm against both variant yamls. Expects the env vars set by
# install_deps.sh (DISTRO_TAG, PKG_FORMAT, GRPC_RUNTIME, PROTOBUF_RUNTIME)
# plus VERSION (set by the workflow from the git tag).
#
# Output: dist/*.deb or dist/*.rpm with distro-tagged filenames.

set -eu

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "${REPO_ROOT}"

: "${VERSION:?VERSION must be set (workflow extracts from the git tag)}"
: "${DISTRO_TAG:?DISTRO_TAG must be set (install_deps.sh did not run?)}"
: "${PKG_FORMAT:?PKG_FORMAT must be set (deb or rpm)}"
: "${GRPC_RUNTIME:?GRPC_RUNTIME must be set}"
: "${PROTOBUF_RUNTIME:?PROTOBUF_RUNTIME must be set}"

ARCH="${ARCH:-amd64}"   # nfpm normalises amd64/x86_64 internally for rpm.

mkdir -p dist
export VERSION ARCH GRPC_RUNTIME PROTOBUF_RUNTIME

for variant in standalone library; do
    case "${variant}" in
        standalone) name=mdt-dialout-collector ;;
        library)    name=mdt-dialout-collector-lib ;;
    esac
    out="dist/${name}_${VERSION}_${DISTRO_TAG}_${ARCH}.${PKG_FORMAT}"
    echo "==> nfpm pkg ${variant} → ${out}"
    nfpm pkg --packager "${PKG_FORMAT}" \
        --config "pkg/nfpm.${variant}.yaml" \
        --target "${out}"
done

echo "==> dist/"
ls -lh dist/
