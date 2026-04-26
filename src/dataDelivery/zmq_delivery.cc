// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#include "zmq_delivery.h"
#include "../bridge/grpc_collector_bridge.h"
#include <zmq.hpp>


ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor: ZmqDelivery()");
    this->set_zmq_transport_uri();
}

bool ZmqPush::ZmqPusher(
    DataWrapper &data_wrapper,
    zmq::socket_t &zmq_sock,
    const std::string &zmq_transport_uri)
{
    grpc_payload *pload;

    InitGrpcPayload(
        &pload,
        data_wrapper.get_event_type().c_str(),
        data_wrapper.get_serialization().c_str(),
        data_wrapper.get_writer_id().c_str(),
        data_wrapper.get_telemetry_node().c_str(),
        data_wrapper.get_telemetry_port().c_str(),
        data_wrapper.get_telemetry_data().c_str());

    // Push only the pointer; the consumer owns the underlying alloc.
    const size_t size = sizeof(grpc_payload *);
    zmq::message_t message(&pload, size);

    try {
        auto sent = zmq_sock.send(message, zmq::send_flags::dontwait);
        if (!sent.has_value()) {
            // EAGAIN under dontwait: consumer can't keep up. Free here —
            // the consumer never received the pointer.
            free_grpc_payload(pload);
            spdlog::get("multi-logger")->
                warn("[ZmqPusher] data-delivery: dropped "
                "(consumer not ready)");
            return false;
        }
        spdlog::get("multi-logger")->
            info("[ZmqPusher] data-delivery: "
                "message successfully sent");
    } catch(const zmq::error_t &zex) {
        free_grpc_payload(pload);
        spdlog::get("multi-logger")->
            error("[ZmqPusher] data-delivery issue: "
            "{}", zex.what());
        return false;
    }

    return true;
}

void ZmqPull::ZmqPoller(
    zmq::socket_t &zmq_sock,
    const std::string &zmq_transport_uri)
{
    const size_t size = sizeof(grpc_payload *);
    zmq::message_t message(size);

    try {
        auto res = zmq_sock.recv(message, zmq::recv_flags::none);
        if (res.value() != 0) {
            spdlog::get("multi-logger")->
                info("[ZmqPoller] data-delivery: "
                    "message successfully received");
            grpc_payload *pload = *(grpc_payload **) message.data();
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
            free_grpc_payload(pload);
        }
    } catch(const zmq::error_t &zex) {
        // ETERM = parent ctx shut down; re-throw so the thread can exit.
        if (zex.num() == ETERM) {
            throw;
        }
        // Transient zmq errors: log and continue rather than killing the
        // entire daemon — the outer poller loop will retry.
        spdlog::get("multi-logger")->
            error("[ZmqPoller] data-delivery issue: "
            "{}", zex.what());
    }
}

