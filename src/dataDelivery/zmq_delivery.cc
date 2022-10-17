// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"


ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor::ZmqDelivery()");
}

void ZmqDelivery::ZmqPusher(std::string &payload)
{
	zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);

    std::string sok = "ipc:///tmp/grpc.sock";
    try {
        sock.connect(sok);
        sock.send(zmq::buffer(payload));
        //sock.send(zmq::buffer(payload), zmq::send_flags::dontwait);
    } catch(zmq::error_t &e) {
        std::cout << "ZMQ Exception: " << e.what() << std::endl;
    }
}

