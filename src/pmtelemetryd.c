// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "bridge/grpc_collector_bridge.h"


int main(int argc, char *argv[])
{
    grpc_payload *pload = NULL;
    start_grpc_dialout_collector(
        "/etc/opt/mdt-dialout-collector/mdt_dialout_collector.conf");

    void *ctx = zmq_ctx_new();
    void *zmq_pull = zmq_socket(ctx, ZMQ_PULL);
    zmq_bind(zmq_pull, "ipc:///tmp/grpc.sock");
    //zmq_bind(zmq_pull, "inproc://grpc");

    while(1) {
        zmq_recv(zmq_pull, &pload, sizeof(grpc_payload), 0);
        printf("%s\n", pload->writer_id);
        printf("%s\n", pload->event_type);
        printf("%s\n", pload->serialization);
        printf("%s\n", pload->telemetry_node);
        printf("%s\n", pload->telemetry_port);
        printf("%s\n", pload->telemetry_data);
        free_grpc_payload(pload);
    }

    return EXIT_SUCCESS;
}

