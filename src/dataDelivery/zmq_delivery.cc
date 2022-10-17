// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"


ZmqDelivery::ZmqDelivery()
{
    spdlog::get("multi-logger")->debug("constructor::ZmqDelivery()");
}

