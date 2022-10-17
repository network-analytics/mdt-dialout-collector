// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _ZMQ_DELIVERY_H_
#define _ZMQ_DELIVERY_H_

// C++ Standard Library headers

// External Library headers
#include "zmq/zmq.hpp"
#include "zmq/zmq_addon.hpp"
// mdt-dialout-collector Library headers
#include "cfg_handler.h"
#include "logs_handler.h"


class ZmqDelivery {
public:
    ZmqDelivery();
    ~ZmqDelivery() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqDelivery()"); };
private:
};

#endif

