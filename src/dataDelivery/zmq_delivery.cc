// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"
#include "../bridge/bridge_lib.h"


ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor::ZmqDelivery()");
    //this->set_zmq_stransport_uri("ipc:///tmp/grpc.sock");
    this->set_zmq_stransport_uri("inproc://grpc");
}

bool ZmqDelivery::ZmqPusher(
    DataWrapper &data_wrapper,
    zmq::context_t &zmq_ctx,
    const std::string &zmq_transport_uri)
{
    Payload *pload;

    InitPayload(
        &pload,
        data_wrapper.get_event_type().c_str(),
        data_wrapper.get_serialization().c_str(),
        data_wrapper.get_writer_id().c_str(),
        data_wrapper.get_telemetry_node().c_str(),
        data_wrapper.get_telemetry_port().c_str(),
        data_wrapper.get_telemetry_data().c_str());

    // Message Buff preparation
    // PUSH-ing only the pointer to the data-struct
    const size_t size = sizeof(Payload *);
    zmq::message_t message(&pload, size);

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
    const size_t size = sizeof(Payload *);
    zmq::message_t message(size);

    zmq::socket_t sock(zmq_ctx, zmq::socket_type::pull);
    sock.bind(zmq_transport_uri);

    try {
        auto res = sock.recv(message, zmq::recv_flags::none);
        if (res.value() != 0) {
            Payload *pload = *(Payload **) message.data();
            std::cout << "PULL-ing from " << zmq_transport_uri << ": "
                << pload->event_type
                << " "
                << pload->serialization
                << " "
                << pload->writer_id
                << " "
                << pload->telemetry_node
                << " "
                << pload->telemetry_port
                << " "
                << pload->telemetry_data
                << "\n";
            FreePayload(pload);
        }
        //std::chrono::milliseconds(100);
    } catch(const zmq::error_t &zex) {
        spdlog::get("multi-logger")->
            error("[ZmqPoller] data-delivery issue: "
            "{}", zex.what());
        std::exit(EXIT_FAILURE);
    }
}

