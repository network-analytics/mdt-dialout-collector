// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _ZMQ_DELIVERY_H_
#define _ZMQ_DELIVERY_H_

// C++ Standard Library headers

// External Library headers
#include <zmq.hpp>
#include <zmq_addon.hpp>
// mdt-dialout-collector Library headers
#include "cfg_handler.h"
#include "logs_handler.h"


class ZmqDelivery {
public:
    ZmqDelivery();
    ~ZmqDelivery() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqDelivery()"); };
    bool ZmqPusher(
        std::string &payload,
        zmq::context_t &zmq_ctx,
        const std::string &zmq_transport_uri);
    void set_zmq_stransport_uri(const std::string &zmq_transport_uri) {
        this->zmq_transport_uri = zmq_transport_uri; };
    std::string get_zmq_stransport_uri() {
        return zmq_transport_uri; };
    zmq::context_t &get_zmq_ctx() {
        return zmq_ctx; };
private:
    zmq::context_t zmq_ctx;
    std::string zmq_transport_uri;
};

#endif

