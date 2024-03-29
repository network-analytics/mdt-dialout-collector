// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _ZMQ_DELIVERY_H_
#define _ZMQ_DELIVERY_H_

// C++ Standard Library headers

// External Library headers
#include <zmq.hpp>
#include <zmq_addon.hpp>
// mdt-dialout-collector Library headers
#include "../dataWrapper/data_wrapper.h"
#include "../utils/cfg_handler.h"
#include "../utils/logs_handler.h"


class ZmqDelivery {
public:
    ZmqDelivery();
    ~ZmqDelivery() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqDelivery()"); };
    void set_zmq_transport_uri() {
        this->zmq_transport_uri = zmq_delivery_cfg_parameters.at("zmq_uri"); };
    const std::string &get_zmq_transport_uri() {
        return this->zmq_transport_uri; };
private:
    std::string zmq_transport_uri;
};

class ZmqPush : public ZmqDelivery {
public:
    ZmqPush() { spdlog::get("multi-logger")->
        debug("constructor: ZmqPush()"); };
    ~ZmqPush() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqPush()"); };
    bool ZmqPusher(
        DataWrapper &data_wrapper,
        zmq::socket_t &zmq_sock,
        const std::string &zmq_transport_uri);
    zmq::context_t &get_zmq_ctx() {
        return this->zmq_ctx; };
private:
    zmq::context_t zmq_ctx;
};

class ZmqPull : public ZmqDelivery {
public:
    ZmqPull() { spdlog::get("multi-logger")->
        debug("constructor: ZmqPull()"); };
    ~ZmqPull() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqPull()"); };
    void ZmqPoller(
        zmq::socket_t &zmq_sock,
        const std::string &zmq_transport_uri);
    zmq::context_t &get_zmq_ctx() {
        return this->zmq_ctx; };
private:
    zmq::context_t zmq_ctx;
};

#endif

