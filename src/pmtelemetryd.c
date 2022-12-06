// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "bridge/bridge_lib.h"


int main(int argc, char *argv[])
{
    Payload *pload = NULL;
    pthread_t *worker = NULL;
    start_grpc_dialout_collector(worker);

<<<<<<< HEAD
    void *ctx = zmq_ctx_new();
    void *zmq_pull = zmq_socket(ctx, ZMQ_PULL);
    zmq_bind(zmq_pull, "ipc:///tmp/grpc.sock");
    //zmq_bind(zmq_pull, "inproc://grpc");

    while(1) {
        zmq_recv(zmq_pull, &pload, 10, 0);
        printf("%s\n", pload->writer_id);
        printf("%s\n", pload->event_type);
        printf("%s\n", pload->serialization);
        printf("%s\n", pload->telemetry_node);
        printf("%s\n", pload->telemetry_port);
        printf("%s\n", pload->telemetry_data);
        FreePayload(pload);
    }

||||||| 434f61c
=======
    void *ctx = zmq_ctx_new();
    void *zmq_pull = zmq_socket(ctx, ZMQ_PULL);
    zmq_bind(zmq_pull, "ipc:///tmp/grpc.sock");
    //zmq_bind(zmq_pull, "inproc://grpc");

    while(1) {
        zmq_recv(zmq_pull, &pload, sizeof(Payload), 0);
        printf("%s\n", pload->writer_id);
        printf("%s\n", pload->event_type);
        printf("%s\n", pload->serialization);
        printf("%s\n", pload->telemetry_node);
        printf("%s\n", pload->telemetry_port);
        printf("%s\n", pload->telemetry_data);
        FreePayload(pload);
    }

>>>>>>> zmqpr
    return EXIT_SUCCESS;
}

