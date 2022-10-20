// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _ZMQ_DELIVERY_H_
#define _ZMQ_DELIVERY_H_

// C++ Standard Library headers

// External Library headers
#include <zmq.hpp>
#include <zmq_addon.hpp>
// mdt-dialout-collector Library headers
#include "data_wrapper.h"
#include "cfg_handler.h"
#include "logs_handler.h"


#define MAX_BUF 4096

typedef struct {
    char event_type[MAX_BUF];
    //const char *event_type;
    char serialization[MAX_BUF];
    //const char *serialization;
    char writer_id[MAX_BUF];
    //const char *writer_id;
    //const char *telemetry_node;
    //const char *telemetry_port;
    //const char *telemetry_data;
} payload;

class ZmqDelivery {
public:
    ZmqDelivery();
    ~ZmqDelivery() { spdlog::get("multi-logger")->
        debug("destructor: ~ZmqDelivery()"); };
    bool ZmqPusher(
        zmq::context_t &zmq_ctx,
        const std::string &zmq_transport_uri);
    void ZmqPoller(
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

extern payload *pload;

extern void set_payload(
    ZmqDelivery &zmq_delivery,
    DataWrapper &dwrapper);

#endif

