#!/usr/bin/env bash
# End-to-end test: synthetic gRPC dial-out client → collector → Kafka, for
# every supported vendor in burst mode.
#
# For each vendor we open one gRPC stream and emit COUNT messages spaced
# INTERVAL seconds apart, each carrying a unique canary suffix
# (<base>-1, <base>-2, ...). After the burst, we assert that *all* N
# canaries appear in the kafka topic the collector publishes to.
#
# Pass --keep to leave containers running for inspection on failure.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
NETWORK="${NETWORK:-mdt-e2e}"
TOPIC="${TOPIC:-mdt-e2e}"
COUNT="${COUNT:-5}"
INTERVAL="${INTERVAL:-1}"
KEEP=0
COLLECTOR_CF="${REPO_ROOT}/tests/e2e/collector/Containerfile"
COLLECTOR_TAG="mdt-e2e-collector"
COLLECTOR_MODE="standalone"  # standalone | pmtelemetryd
COLLECTOR_CONF="${REPO_ROOT}/tests/e2e/collector/mdt_dialout_collector.conf"
TLS=0
TLS_CA_HOST=""
PKG_PATH=""              # --package PATH
PKG_DISTRO_OVERRIDE=""   # --distro IMAGE (overrides filename auto-detect)
PKG_STAGE_DIR=""         # set later when --package is used

while [ $# -gt 0 ]; do
    arg="$1"
    case "$arg" in
        --keep) KEEP=1 ;;
        --next)
            # Build against the newer gRPC/protobuf/etc. dep stack.
            COLLECTOR_CF="${REPO_ROOT}/tests/e2e/collector/Containerfile.next"
            COLLECTOR_TAG="mdt-e2e-collector-next"
            ;;
        --pmtelemetryd)
            # Test the LIBRARY path: pmtelemetryd embeds libgrpc_collector
            # via ZMQ, then writes to kafka itself.
            COLLECTOR_CF="${REPO_ROOT}/tests/e2e/pmtelemetryd/Containerfile"
            COLLECTOR_TAG="mdt-e2e-pmtelemetryd"
            COLLECTOR_MODE="pmtelemetryd"
            TOPIC="mdt-e2e-pmtd"
            ;;
        --tls)
            # Server-side TLS variant: collector image bakes a self-signed
            # cert and the .tls.conf turns enable_tls on. Clients receive the
            # CA cert via MDT_E2E_TLS_CA and connect on a secure channel.
            COLLECTOR_CF="${REPO_ROOT}/tests/e2e/collector/Containerfile.tls"
            COLLECTOR_TAG="mdt-e2e-collector-tls"
            COLLECTOR_CONF="${REPO_ROOT}/tests/e2e/collector/mdt_dialout_collector.tls.conf"
            TLS=1
            ;;
        --ipv6)
            # IPv6 variant: collector binds [::]:port for every vendor.
            # Linux dual-stack default accepts v4-mapped connections too,
            # so the existing v4 clients still reach the daemon — gRPC
            # reports the peer as ipv6:[::ffff:a.b.c.d]:port, exercising
            # the IPv6 branch of ParsePeer().
            COLLECTOR_CONF="${REPO_ROOT}/tests/e2e/collector/mdt_dialout_collector.ipv6.conf"
            ;;
        --package)
            # Installed-package variant: take a prebuilt .deb/.rpm and
            # apt/dnf-install it in a clean container of the matching distro.
            # Catches postinst / runtime-dep / file-layout regressions the
            # source-build path can't see. Argument is a path to the standalone
            # .deb/.rpm (NOT the -lib variant). Distro auto-detected from the
            # filename (_bookworm_/_trixie_/_noble_/_fc_); --distro overrides.
            shift
            PKG_PATH="${1:?--package needs a path to .deb/.rpm}"
            COLLECTOR_CF="${REPO_ROOT}/tests/e2e/collector/Containerfile.pkg"
            COLLECTOR_TAG="mdt-e2e-collector-pkg"
            ;;
        --distro)
            shift
            PKG_DISTRO_OVERRIDE="${1:?--distro needs an image name}"
            ;;
        *) echo "unknown arg: $arg" >&2; exit 2 ;;
    esac
    shift
done

VENDORS=(
    "cisco:10007:send_cisco.py"
    "juniper:10008:send_juniper.py"
    "nokia:10009:send_nokia.py"
    "huawei:10010:send_huawei.py"
)

cleanup() {
    if [ "${KEEP}" -eq 1 ]; then
        echo
        echo "--- containers left running (per --keep) ---"
        podman ps --filter "network=${NETWORK}"
        return
    fi
    echo
    echo "--- cleanup ---"
    for c in mdt-e2e-client mdt-e2e-collector mdt-e2e-broker; do
        podman rm -f "$c" >/dev/null 2>&1 || true
    done
    podman network rm "${NETWORK}" >/dev/null 2>&1 || true
    if [ -n "${TLS_CA_HOST}" ] && [ -d "$(dirname "${TLS_CA_HOST}")" ]; then
        rm -rf "$(dirname "${TLS_CA_HOST}")"
    fi
    if [ -n "${PKG_STAGE_DIR}" ] && [ -d "${PKG_STAGE_DIR}" ]; then
        rm -rf "${PKG_STAGE_DIR}"
    fi
}
trap cleanup EXIT

# --package staging: copy the .deb/.rpm into the build context (podman build
# COPY needs the source under the context root) and pick a base image.
PKG_BUILD_ARGS=()
if [ -n "${PKG_PATH}" ]; then
    test -f "${PKG_PATH}" \
        || { echo "--package: file not found: ${PKG_PATH}" >&2; exit 2; }
    PKG_BASENAME="$(basename "${PKG_PATH}")"
    PKG_STAGE_DIR="${REPO_ROOT}/tests/e2e/.pkg-stage"
    mkdir -p "${PKG_STAGE_DIR}"
    cp "${PKG_PATH}" "${PKG_STAGE_DIR}/${PKG_BASENAME}"
    PKG_REL="tests/e2e/.pkg-stage/${PKG_BASENAME}"
    if [ -n "${PKG_DISTRO_OVERRIDE}" ]; then
        PKG_DISTRO="${PKG_DISTRO_OVERRIDE}"
    else
        case "${PKG_BASENAME}" in
            *_bookworm_*)  PKG_DISTRO="debian:12" ;;
            *_trixie_*)    PKG_DISTRO="debian:13" ;;
            *_noble_*)     PKG_DISTRO="ubuntu:24.04" ;;
            *_fc_*|*.rpm)  PKG_DISTRO="fedora:latest" ;;
            *)             PKG_DISTRO="debian:12" ;;
        esac
    fi
    echo "=== package: ${PKG_BASENAME} → ${PKG_DISTRO} ==="
    PKG_BUILD_ARGS=(
        --build-arg "DISTRO=${PKG_DISTRO}"
        --build-arg "PKG_FILE=${PKG_REL}"
    )
fi

echo "=== build images (collector tag=${COLLECTOR_TAG}) ==="
podman build "${PKG_BUILD_ARGS[@]}" -t "${COLLECTOR_TAG}" -f "${COLLECTOR_CF}" "${REPO_ROOT}"
podman build -t mdt-e2e-client \
    -f "${REPO_ROOT}/tests/e2e/client/Containerfile" "${REPO_ROOT}"

if [ "${TLS}" -eq 1 ]; then
    # Pull the self-signed cert out of the freshly built image so the
    # client container can mount it as the trust anchor.
    TLS_TMPDIR="$(mktemp -d -t mdt-e2e-tls.XXXXXX)"
    TLS_CA_HOST="${TLS_TMPDIR}/server.crt"
    cid="$(podman create "${COLLECTOR_TAG}")"
    podman cp "${cid}:/etc/mdt-tls/server.crt" "${TLS_CA_HOST}"
    podman rm "${cid}" >/dev/null
    echo "extracted server cert to ${TLS_CA_HOST}"
fi

echo
echo "=== network ==="
podman network exists "${NETWORK}" || podman network create "${NETWORK}"

echo
echo "=== broker (redpanda) ==="
podman run -d --rm --name mdt-e2e-broker \
    --network "${NETWORK}" --network-alias broker \
    docker.redpanda.com/redpandadata/redpanda:v24.2.7 \
    redpanda start --overprovisioned --smp 1 --memory 512M \
        --reserve-memory 0M --node-id 0 --check=false \
        --kafka-addr PLAINTEXT://0.0.0.0:9092 \
        --advertise-kafka-addr PLAINTEXT://broker:9092 >/dev/null
echo "waiting for broker ..."
for i in $(seq 1 30); do
    if podman exec mdt-e2e-broker rpk cluster health 2>/dev/null \
        | grep -q "Healthy:.*true"; then
        break
    fi
    sleep 1
done

echo "creating topic ${TOPIC} ..."
podman exec mdt-e2e-broker rpk topic create "${TOPIC}" --partitions 1 \
    --replicas 1 >/dev/null 2>&1 || true

echo
echo "=== collector (${COLLECTOR_MODE}) ==="
if [ "${COLLECTOR_MODE}" = "pmtelemetryd" ]; then
    podman run -d --rm --name mdt-e2e-collector \
        --network "${NETWORK}" --network-alias collector \
        -v "${REPO_ROOT}/tests/e2e/pmtelemetryd:/etc/pmacct:ro,Z" \
        "${COLLECTOR_TAG}" >/dev/null
else
    podman run -d --rm --name mdt-e2e-collector \
        --network "${NETWORK}" --network-alias collector \
        -v "${COLLECTOR_CONF}:/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf:ro,Z" \
        "${COLLECTOR_TAG}" >/dev/null
fi
echo "waiting for collector ports ..."
sleep 3

declare -A BASE_CANARIES

for spec in "${VENDORS[@]}"; do
    IFS=':' read -r vendor port script <<<"${spec}"
    base="e2e-canary-${vendor}-${RANDOM}"
    BASE_CANARIES["${vendor}"]="${base}"

    echo
    echo "=== ${vendor}: stream ${COUNT} messages every ${INTERVAL}s "
    echo "    (base canary='${base}') ==="
    tls_args=()
    if [ "${TLS}" -eq 1 ]; then
        tls_args=(
            -v "${TLS_CA_HOST}:/etc/mdt-tls/server.crt:ro,Z"
            -e "MDT_E2E_TLS_CA=/etc/mdt-tls/server.crt"
        )
    fi
    podman run --rm --name "mdt-e2e-client" \
        --network "${NETWORK}" \
        --entrypoint "python3" \
        "${tls_args[@]}" \
        mdt-e2e-client \
        "${script}" \
        --target "collector:${port}" \
        --sensor-value "${base}" \
        --count "${COUNT}" \
        --interval "${INTERVAL}" || {
            echo "client run failed for ${vendor}"
            BASE_CANARIES["${vendor}__FAIL"]=1
        }
done

# Sleep is just so any in-flight gRPC half-close has time to land at the
# collector; the actual "wait for kafka" happens via rpk --num below.
sleep 1

echo
echo "=== consume from kafka and assert per-vendor canaries present ==="
expected=$(( ${#VENDORS[@]} * COUNT ))
# --num bounds the wait: rpk returns early once it has $expected messages.
# 30s is generous: covers librdkafka batching delays under load (linger.ms
# defaults are larger in 2.x) without hanging forever if the daemon stalls.
out="$(podman exec mdt-e2e-broker bash -c \
    "timeout 30 rpk topic consume ${TOPIC} --num ${expected} --offset start --format '%v\n' 2>/dev/null" \
    || true)"
echo "${out}"

echo
fail_vendors=()
total_expected=0
total_found=0
for spec in "${VENDORS[@]}"; do
    IFS=':' read -r vendor _ _ <<<"${spec}"
    base="${BASE_CANARIES[${vendor}]}"
    found=0
    missing=()
    for i in $(seq 1 "${COUNT}"); do
        canary="${base}-${i}"
        total_expected=$((total_expected + 1))
        if grep -qF -- "${canary}" <<< "${out}"; then
            found=$((found + 1))
            total_found=$((total_found + 1))
        else
            missing+=("${i}")
        fi
    done
    if [ "${found}" -eq "${COUNT}" ]; then
        echo "PASS: ${vendor} ${found}/${COUNT} canaries (base='${base}')"
    else
        echo "FAIL: ${vendor} ${found}/${COUNT} canaries — missing seqs: ${missing[*]}"
        fail_vendors+=("${vendor}")
    fi
done

echo
if [ "${#fail_vendors[@]}" -eq 0 ]; then
    echo "=== ALL ${#VENDORS[@]} VENDORS PASSED (${total_found}/${total_expected} messages) ==="
    exit 0
else
    echo "=== FAILED: ${fail_vendors[*]} (${total_found}/${total_expected}) ==="
    echo
    echo "--- collector logs (last 80 lines) ---"
    podman logs mdt-e2e-collector 2>&1 | tail -80
    exit 1
fi
