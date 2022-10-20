// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "zmq_delivery.h"


payload *pload = (payload *) malloc(sizeof(payload *));

void set_payload(
    ZmqDelivery &zmq_delivery,
    DataWrapper &dwrapper)
{
    strcpy(
        pload->event_type,
        dwrapper.get_event_type().c_str());
    //pload->event_type = dwrapper.get_event_type().c_str();
    strcpy(
        pload->serialization,
        dwrapper.get_serialization().c_str());
    //pload->serialization = dwrapper.get_serialization().c_str();
    //pload->writer_id = dwrapper.get_writer_id().c_str();
    strcpy(
        pload->writer_id,
        dwrapper.get_writer_id().c_str());
    //pload->telemetry_node = dwrapper.get_telemetry_node().c_str();
    //pload->telemetry_port = dwrapper.get_telemetry_port().c_str();
    //pload->telemetry_data = dwrapper.get_telemetry_data().c_str();

    zmq_delivery.ZmqPusher(
        zmq_delivery.get_zmq_ctx(),
        zmq_delivery.get_zmq_stransport_uri());

    zmq_delivery.ZmqPoller(
        zmq_delivery.get_zmq_ctx(),
        zmq_delivery.get_zmq_stransport_uri());
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
    const size_t size = 8192;
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
    const size_t size = 8192;
    zmq::message_t message(size);

    zmq::socket_t sock(zmq_ctx, zmq::socket_type::pull);
    sock.bind(zmq_transport_uri);

    try {
    //    while(true) {
            auto res = sock.recv(message, zmq::recv_flags::none);
            if (res.value() != 0) {
                std::cout << "PULL-ing from " << zmq_transport_uri << ": "
                    << static_cast<payload *>(message.data())->event_type
                    << " "
                    << static_cast<payload *>(message.data())->serialization
                    << " "
                    << static_cast<payload *>(message.data())->writer_id
                    << "\n ";
                    //<< static_cast<payload *>(message.data())->telemetry_node
                    //<< " "
                    //<< static_cast<payload *>(message.data())->telemetry_port
                    //<< " "
                    //<< static_cast<payload *>(message.data())->telemetry_data
                    //<< "\n";
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

