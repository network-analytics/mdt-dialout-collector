"""
Synthetic Juniper gNMI dial-out client — burst mode.

Opens ONE Subscriber.DialOutSubscriber stream and yields N SubscribeResponse
messages over it, each carrying a unique canary in a prefix path key.

Set MDT_E2E_TLS_CA to the path of the server's CA cert to switch to a TLS
gRPC channel; otherwise an insecure channel is used.
"""
import argparse
import os
import sys
import threading
import time

import grpc

import juniper_dialout_pb2_grpc
import juniper_gnmi_pb2


def _make_channel(target: str) -> grpc.Channel:
    ca_path = os.environ.get("MDT_E2E_TLS_CA")
    if ca_path:
        with open(ca_path, "rb") as f:
            ca_pem = f.read()
        creds = grpc.ssl_channel_credentials(root_certificates=ca_pem)
        opts = (("grpc.ssl_target_name_override", "collector"),)
        return grpc.secure_channel(target, creds, options=opts)
    return grpc.insecure_channel(target)


def build_subscribe_response(canary: str
                             ) -> juniper_gnmi_pb2.SubscribeResponse:
    resp = juniper_gnmi_pb2.SubscribeResponse()
    notif = resp.update
    notif.timestamp = int(time.time() * 1_000_000_000)

    notif.prefix.origin = "openconfig-interfaces"
    iface_elem = notif.prefix.elem.add()
    iface_elem.name = "interface"
    iface_elem.key["name"] = canary

    upd = notif.update.add()
    leaf = upd.path.elem.add()
    leaf.name = "state"
    leaf2 = upd.path.elem.add()
    leaf2.name = "description"
    upd.val.string_val = canary
    return resp


def stream_burst(target: str, base_canary: str, count: int,
                 interval: float) -> None:
    channel = _make_channel(target)
    grpc.channel_ready_future(channel).result(timeout=10)
    stub = juniper_dialout_pb2_grpc.SubscriberStub(channel)

    drained = threading.Event()

    def gen():
        for i in range(count):
            canary_n = f"{base_canary}-{i + 1}"
            print(f"  juniper msg {i + 1}/{count} canary={canary_n}",
                  flush=True)
            yield build_subscribe_response(canary_n)
            if i < count - 1:
                time.sleep(interval)
        drained.set()

    call = stub.DialOutSubscriber(gen())
    drained.wait(timeout=count * interval + 10)
    time.sleep(0.5)
    call.cancel()
    channel.close()


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--target", default="collector:10008")
    p.add_argument("--sensor-value", default="e2e-canary")
    p.add_argument("--count", type=int, default=5)
    p.add_argument("--interval", type=float, default=1.0)
    args = p.parse_args()

    print(f"juniper: streaming {args.count} messages to {args.target} "
          f"@ {args.interval}s interval (base canary='{args.sensor_value}')")
    stream_burst(args.target, args.sensor_value, args.count, args.interval)
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
