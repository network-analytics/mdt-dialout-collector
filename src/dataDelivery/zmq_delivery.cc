// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"


Payload *pload = (Payload *) malloc(sizeof(Payload));

void InitPayload(Payload *pload, const char *event_type,
    const char *serialization, const char *writer_id,
    const char *telemetry_node, const char *telemetry_port,
    const char *telemetry_data)
{
    size_t length_event_type = (strlen(event_type) + 1);
    size_t length_serialization = (strlen(serialization) + 1);
    size_t length_writer_id = (strlen(writer_id) + 1);
    size_t length_telemetry_node = (strlen(telemetry_node) + 1);
    size_t length_telemetry_port = (strlen(telemetry_port) + 1);
    size_t length_telemetry_data = (strlen(telemetry_data) + 1);

    pload->event_type = (char *) malloc(length_event_type);
    memset(pload->event_type, 0, (length_event_type * sizeof(char)));
    strncpy(pload->event_type, event_type, length_event_type);

    pload->serialization = (char *) malloc(length_serialization);
    memset(pload->serialization, 0, (length_serialization * sizeof(char)));
    strncpy(pload->serialization, serialization, length_serialization);

    pload->writer_id = (char *) malloc(length_writer_id);
    memset(pload->writer_id, 0, (length_writer_id * sizeof(char)));
    strncpy(pload->writer_id, writer_id, length_writer_id);

    pload->telemetry_node = (char *) malloc(length_telemetry_node);
    memset(pload->telemetry_node, 0, (length_telemetry_node * sizeof(char)));
    strncpy(pload->telemetry_node, telemetry_node, length_telemetry_node);

    pload->telemetry_port = (char *) malloc(length_telemetry_port);
    memset(pload->telemetry_port, 0, (length_telemetry_port * sizeof(char)));
    strncpy(pload->telemetry_port, telemetry_port, length_telemetry_port);

    pload->telemetry_data = (char *) malloc(length_telemetry_data);
    memset(pload->telemetry_data, 0, (length_telemetry_data * sizeof(char)));
    strncpy(pload->telemetry_data, telemetry_data, length_telemetry_data);
}

void FreePayload(Payload *pload)
{
    free(pload->event_type);
    free(pload->serialization);
    free(pload->writer_id);
    free(pload->telemetry_node);
    free(pload->telemetry_port);
    free(pload->telemetry_data);
}

ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor::ZmqDelivery()");
    this->set_zmq_stransport_uri("ipc:///tmp/grpc.sock");
    //this->set_zmq_stransport_uri("inproc://grpc");
}

bool ZmqDelivery::ZmqPusher(
    zmq::context_t &zmq_ctx,
    const std::string &zmq_transport_uri)
{
    // Message Buff preparation
    // PUSH-ing only the pointer to the data-struct
    const size_t size = sizeof(Payload);
    zmq::message_t message(pload, size);

    zmq::socket_t sock(zmq_ctx, zmq::socket_type::push);
    sock.connect(zmq_transport_uri);

    try {
        //sock.send(zmq::buffer(payload), zmq::send_flags::none);
        sock.send(message, zmq::send_flags::none);
        spdlog::get("multi-logger")->
            info("[ZmqPusher] data-delivery: "
                "message successfully delivered");
    } catch(const zmq::error_t &zex) {
        spdlog::get("multi-logger")->
            error("[ZmqPusher] data-delivery issue: "
            "{}", zex.what());
        return false;
    }

    return true;
}

void ZmqDelivery::ZmqPoller(
    zmq::context_t &zmq_ctx,
    const std::string &zmq_transport_uri)
{
    // Message Buff preparation
    // POLL-ing only the pointer to the data-struct
    const size_t size = sizeof(Payload);
    zmq::message_t message(size);

    zmq::socket_t sock(zmq_ctx, zmq::socket_type::pull);
    sock.bind(zmq_transport_uri);

    try {
    //    while(true) {
            auto res = sock.recv(message, zmq::recv_flags::none);
            if (res.value() != 0) {
                std::cout << "PULL-ing from " << zmq_transport_uri << ": "
                    << static_cast<Payload *>(message.data())->event_type
                    << " "
                    << static_cast<Payload *>(message.data())->serialization
                    << " "
                    << static_cast<Payload *>(message.data())->writer_id
                    << " "
                    << static_cast<Payload *>(message.data())->telemetry_node
                    << " "
                    << static_cast<Payload *>(message.data())->telemetry_port
                    << " "
                    << static_cast<Payload *>(message.data())->telemetry_data
                    << "\n";
            }
            //std::chrono::milliseconds(100);
    //    }
    } catch(const zmq::error_t &zex) {
        spdlog::get("multi-logger")->
            error("[ZmqPoller] data-delivery issue: "
            "{}", zex.what());
        std::exit(EXIT_FAILURE);
    }
}

