// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"


ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor::ZmqDelivery()");
    this->set_zmq_stransport_uri("ipc:///tmp/grpc.sock");
}

bool ZmqDelivery::ZmqPusher(
    std::string &payload,
    zmq::context_t &zmq_ctx,
    const std::string &zmq_transport_uri)
{
    zmq::socket_t sock(zmq_ctx, zmq::socket_type::push);

    sock.connect(zmq_transport_uri);

    try {
        sock.send(zmq::buffer(payload));
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

