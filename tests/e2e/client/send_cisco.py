"""
Synthetic Cisco MDT dial-out client — burst mode.

Opens ONE bidi stream and emits N MdtDialoutArgs over it, each with a unique
canary suffix, sleeping --interval seconds between sends. Mirrors how a real
router behaves (one persistent stream, periodic telemetry).

Set MDT_E2E_TLS_CA to the path of the server's CA cert to switch to a TLS
gRPC channel; otherwise an insecure channel is used.
"""
import argparse
import os
import sys
import threading
import time

import grpc

import cisco_dialout_pb2
import cisco_dialout_pb2_grpc
import cisco_telemetry_pb2


def _make_channel(target: str) -> grpc.Channel:
    ca_path = os.environ.get("MDT_E2E_TLS_CA")
    if ca_path:
        with open(ca_path, "rb") as f:
            ca_pem = f.read()
        creds = grpc.ssl_channel_credentials(root_certificates=ca_pem)
        # Server cert SAN is "collector"; override target name so SNI matches
        # even if the user passed an IP.
        opts = (("grpc.ssl_target_name_override", "collector"),)
        return grpc.secure_channel(target, creds, options=opts)
    return grpc.insecure_channel(target)


def build_payload(canary: str, ts_ms: int) -> bytes:
    tlm = cisco_telemetry_pb2.Telemetry()
    tlm.node_id_str = "e2e-router-1"
    tlm.subscription_id_str = "e2e-subscription"
    tlm.encoding_path = "Cisco-IOS-XR-e2e-test:e2e/probe"
    tlm.collection_id = 1
    tlm.msg_timestamp = ts_ms

    field = tlm.data_gpbkv.add()
    field.name = "probe-name"
    field.timestamp = ts_ms
    inner = field.fields.add()
    inner.name = "sensor"
    inner.string_value = canary
    return tlm.SerializeToString()


def stream_burst(target: str, base_canary: str, count: int,
                 interval: float) -> None:
    channel = _make_channel(target)
    grpc.channel_ready_future(channel).result(timeout=10)
    stub = cisco_dialout_pb2_grpc.gRPCMdtDialoutStub(channel)

    drained = threading.Event()

    def gen():
        for i in range(count):
            canary_n = f"{base_canary}-{i + 1}"
            payload = build_payload(canary_n, int(time.time() * 1000))
            print(f"  cisco msg {i + 1}/{count} canary={canary_n}", flush=True)
            yield cisco_dialout_pb2.MdtDialoutArgs(
                ReqId=i + 1, data=payload, errors="")
            if i < count - 1:
                time.sleep(interval)
        drained.set()

    call = stub.MdtDialout(gen())
    # Wait for the generator to actually finish writing to the wire. Without
    # this, an immediate cancel() truncates the request stream.
    drained.wait(timeout=count * interval + 10)
    time.sleep(0.5)  # tail latency: last message in flight
    call.cancel()
    channel.close()


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--target", default="collector:10007")
    p.add_argument("--sensor-value", default="e2e-canary")
    p.add_argument("--count", type=int, default=5)
    p.add_argument("--interval", type=float, default=1.0)
    args = p.parse_args()

    print(f"cisco: streaming {args.count} messages to {args.target} "
          f"@ {args.interval}s interval (base canary='{args.sensor_value}')")
    stream_burst(args.target, args.sensor_value, args.count, args.interval)
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
