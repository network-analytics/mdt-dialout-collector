"""
Synthetic Huawei dial-out client — burst mode.

Opens ONE gRPCDataservice.dataPublish stream and yields N serviceArgs over
it, each with a unique canary in data_json (the unconditional JSON path —
the GPB path is gated on a hard-coded proto_path).

Set MDT_E2E_TLS_CA to the path of the server's CA cert to switch to a TLS
gRPC channel; otherwise an insecure channel is used.
"""
import argparse
import json
import os
import sys
import threading
import time

import grpc

import huawei_dialout_pb2
import huawei_dialout_pb2_grpc


def _make_channel(target: str) -> grpc.Channel:
    ca_path = os.environ.get("MDT_E2E_TLS_CA")
    if ca_path:
        with open(ca_path, "rb") as f:
            ca_pem = f.read()
        creds = grpc.ssl_channel_credentials(root_certificates=ca_pem)
        opts = (("grpc.ssl_target_name_override", "collector"),)
        return grpc.secure_channel(target, creds, options=opts)
    return grpc.insecure_channel(target)


def build_args(canary: str, req_id: int) -> huawei_dialout_pb2.serviceArgs:
    payload = json.dumps({
        "node_id_str": canary,
        "subscription_id_str": "e2e-sub",
        "sensor_path": "huawei-ifm:ifm/interfaces/interface",
    })
    return huawei_dialout_pb2.serviceArgs(
        ReqId=req_id, data_json=payload, errors="")


def stream_burst(target: str, base_canary: str, count: int,
                 interval: float) -> None:
    channel = _make_channel(target)
    grpc.channel_ready_future(channel).result(timeout=10)
    stub = huawei_dialout_pb2_grpc.gRPCDataserviceStub(channel)

    drained = threading.Event()

    def gen():
        for i in range(count):
            canary_n = f"{base_canary}-{i + 1}"
            print(f"  huawei msg {i + 1}/{count} canary={canary_n}",
                  flush=True)
            yield build_args(canary_n, req_id=i + 1)
            if i < count - 1:
                time.sleep(interval)
        drained.set()

    call = stub.dataPublish(gen())
    drained.wait(timeout=count * interval + 10)
    time.sleep(0.5)
    call.cancel()
    channel.close()


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--target", default="collector:10010")
    p.add_argument("--sensor-value", default="e2e-canary")
    p.add_argument("--count", type=int, default=5)
    p.add_argument("--interval", type=float, default=1.0)
    args = p.parse_args()

    print(f"huawei: streaming {args.count} messages to {args.target} "
          f"@ {args.interval}s interval (base canary='{args.sensor_value}')")
    stream_burst(args.target, args.sensor_value, args.count, args.interval)
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
