// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "mdt_dialout_core.h"
#include <zmq.hpp>


// Global visibility to be able to signal the refresh --> CSV from main
std::unordered_map<std::string, std::vector<std::string>> label_map;

void CustomSocketMutator::log_socket_options(int fd) {
    int type, keepalive, reuseport;
    socklen_t len = sizeof(type);

    // Log SO_TYPE
    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) == 0) {
        std::string type_str;
        switch (type) {
            case SOCK_STREAM:
                type_str = "SOCK_STREAM (TCP)";
                break;
            case SOCK_DGRAM:
                type_str = "SOCK_DGRAM (UDP)";
                break;
            case SOCK_RAW:
                type_str = "SOCK_RAW (Raw)";
                break;
            // Add more types if needed
            default:
                type_str = "Unknown Type";
                break;
        }
        spdlog::get("multi-logger")->debug("Socket Type: {}", type_str);
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_TYPE) failed");
    }

    // Log SO_KEEPALIVE
    if (getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, &len) == 0) {
        spdlog::get("multi-logger")->
            debug("SO_KEEPALIVE: {}", keepalive ? "Enabled" : "Disabled");
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_KEEPALIVE) failed");
    }

    // Log SO_REUSEPORT
    if (getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuseport, &len) == 0) {
        spdlog::get("multi-logger")->
            debug("SO_REUSEPORT: {}", reuseport ? "Enabled" : "Disabled");
    } else {
        spdlog::get("multi-logger")->error("getsockopt(SO_REUSEPORT) failed");
    }
}

bool CustomSocketMutator::bindtodevice_socket_mutator(int fd)
{
    int type;
    socklen_t len = sizeof(type);

    const std::string iface = main_cfg_parameters.at("iface");
    const std::string sbc = main_cfg_parameters.at("so_bindtodevice_check");

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
        spdlog::get("multi-logger")->
            error("[CustomSocketMutator()]: Unable to get the "
            "interface type");
        std::abort();
    }

    if (sbc.compare("true") == 0) {
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
            iface.c_str(), strlen(iface.c_str())) != 0) {
            spdlog::get("multi-logger")->
                error("[CustomSocketMutator()]: Unable to bind [{}] "
                "on the configured socket(s)", iface);
            std::abort();
        }
    } else {
        spdlog::get("multi-logger")->
            warn("[CustomSocketMutator()]: SO_BINDTODEVICE "
            "check disabled");
    }

    log_socket_options(fd);

    return true;
}

#define GPR_ICMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
int custom_socket_compare(grpc_socket_mutator *mutator1,
    grpc_socket_mutator *mutator2)
{
    return GPR_ICMP(mutator1, mutator2);
}

void custom_socket_destroy(grpc_socket_mutator *mutator)
{
    gpr_free(mutator);
}

bool custom_socket_mutator_fd(int fd, grpc_socket_mutator *mutator0)
{
    CustomSocketMutator *csm = (CustomSocketMutator *) mutator0;
    return csm->bindtodevice_socket_mutator(fd);
}

// functions before were used to setup a custom vtable (struct)
const grpc_socket_mutator_vtable custom_socket_mutator_vtable =
    grpc_socket_mutator_vtable {
        custom_socket_mutator_fd,
        custom_socket_compare,
        custom_socket_destroy,
        nullptr
    };

CustomSocketMutator::CustomSocketMutator()
{
    spdlog::get("multi-logger")->debug("constructor: CustomSocketMutator()");
    grpc_socket_mutator_init(this, &custom_socket_mutator_vtable);
}

void ServerBuilderOptionImpl::UpdateArguments(
    grpc::ChannelArguments *custom_args)
{
    CustomSocketMutator *csm_ = new CustomSocketMutator();
    custom_args->SetSocketMutator(csm_);
}

void Srv::CiscoBind(std::string cisco_srv_socket)
{
    grpc::ServerBuilder cisco_builder;
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        csbo(new ServerBuilderOptionImpl());
    cisco_builder.SetOption(std::move(csbo));
    // --- Required for socket manipulation ---
    cisco_builder.RegisterService(&cisco_service_);
    cisco_builder.AddListeningPort(cisco_srv_socket,
        grpc::InsecureServerCredentials());
    cisco_cq_ = cisco_builder.AddCompletionQueue();
    cisco_server_ = cisco_builder.BuildAndStart();

    Srv::CiscoFsmCtrl();
}

void Srv::JuniperBind(std::string juniper_srv_socket)
{
    grpc::ServerBuilder juniper_builder;
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        jsbo(new ServerBuilderOptionImpl());
    juniper_builder.SetOption(std::move(jsbo));
    // --- Required for socket manipulation ---
    juniper_builder.RegisterService(&juniper_service_);
    juniper_builder.AddListeningPort(juniper_srv_socket,
        grpc::InsecureServerCredentials());
    juniper_cq_ = juniper_builder.AddCompletionQueue();
    juniper_server_ = juniper_builder.BuildAndStart();

    Srv::JuniperFsmCtrl();
}

void Srv::HuaweiBind(std::string huawei_srv_socket)
{
    grpc::ServerBuilder huawei_builder;
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        hsbo(new ServerBuilderOptionImpl());
    huawei_builder.SetOption(std::move(hsbo));
    // --- Required for socket manipulation ---
    huawei_builder.RegisterService(&huawei_service_);
    huawei_builder.AddListeningPort(huawei_srv_socket,
        grpc::InsecureServerCredentials());
    huawei_cq_ = huawei_builder.AddCompletionQueue();
    huawei_server_ = huawei_builder.BuildAndStart();

    Srv::HuaweiFsmCtrl();
}

void Srv::CiscoFsmCtrl()
{
    auto tid = std::this_thread::get_id();
    std::stringstream stid;
    stid << tid;
    spdlog::get("multi-logger")->debug("Srv::CiscoFsmCtrl() - Thread-ID: {}",
        stid.str());

    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    DataManipulation data_manipulation;
    DataWrapper data_wrapper;
    cisco_telemetry::Telemetry cisco_tlm;

    // Kafka producer
    KafkaDelivery kafka_delivery;
    const kafka::Properties kproperties = kafka_delivery.get_properties();
    kafka::clients::KafkaProducer kafka_producer(kproperties);

    // Zmq pusher
    ZmqPush zmq_pusher;
    const std::string zmq_uri = zmq_pusher.get_zmq_transport_uri();
    zmq::socket_t zmq_sock(zmq_pusher.get_zmq_ctx(),
        zmq::socket_type::push);
    zmq_sock.connect(zmq_uri);

    if (ddm.compare("kafka") != 0) {
        kafka_producer.~KafkaProducer();
        kafka_delivery.~KafkaDelivery();
    } else if (ddm.compare("zmq") != 0) {
        zmq_sock.~socket_t();
        zmq_pusher.~ZmqPush();
    } else {
        spdlog::get("multi-logger")->
            error("[Srv::CiscoFsmCtrl()]: Unable to set the "
            "delivery function");
        std::abort();
    }

    std::unique_ptr<Srv::CiscoStream> cisco_sstream(
        new Srv::CiscoStream(&cisco_service_, cisco_cq_.get()));
    cisco_sstream->Start(label_map, data_manipulation, data_wrapper,
        kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
        cisco_tlm);
    //int cisco_counter {0};
    void *cisco_tag {nullptr};
    bool cisco_ok {false};
    while (true) {
        //std::cout << "Cisco: " << cisco_counter << "\n";
        GPR_ASSERT(cisco_cq_->Next(&cisco_tag, &cisco_ok));
        //GPR_ASSERT(cisco_ok);
        if (cisco_ok == false) {
            spdlog::get("multi-logger")->
                warn("[CiscoFsmCtrl][grpc::CompletionQueue] "
                "unsuccessful event");
            continue;
        }
        static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Start(
            label_map, data_manipulation, data_wrapper, kafka_delivery,
            kafka_producer, zmq_pusher, zmq_sock, zmq_uri, cisco_tlm);
        //cisco_counter++;
    }

    if (ddm.compare("kafka") == 0) {
        kafka_producer.close();
    } else if (ddm.compare("zmq") == 0) {
        zmq_sock.close();
    }
}

void Srv::JuniperFsmCtrl()
{
    auto tid = std::this_thread::get_id();
    std::stringstream stid;
    stid << tid;
    spdlog::get("multi-logger")->debug("Srv::JuniperFsmCtrl() - Thread-ID: {}",
        stid.str());

    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    DataManipulation data_manipulation;
    DataWrapper data_wrapper;
    GnmiJuniperTelemetryHeaderExtension juniper_tlm_hdr_ext;

    // Kafka producer
    KafkaDelivery kafka_delivery;
    const kafka::Properties kproperties = kafka_delivery.get_properties();
    kafka::clients::KafkaProducer kafka_producer(kproperties);

    // Zmq pusher
    ZmqPush zmq_pusher;
    const std::string zmq_uri = zmq_pusher.get_zmq_transport_uri();
    zmq::socket_t zmq_sock(zmq_pusher.get_zmq_ctx(),
        zmq::socket_type::push);
    zmq_sock.connect(zmq_uri);

    if (ddm.compare("kafka") != 0) {
        kafka_producer.~KafkaProducer();
        kafka_delivery.~KafkaDelivery();
    } else if (ddm.compare("zmq") != 0) {
        zmq_sock.~socket_t();
        zmq_pusher.~ZmqPush();
    } else {
        spdlog::get("multi-logger")->
            error("[Srv::CiscoFsmCtrl()]: Unable to set the "
            "delivery function");
        std::abort();
    }

    std::unique_ptr<Srv::JuniperStream> juniper_sstream(
        new Srv::JuniperStream(&juniper_service_, juniper_cq_.get()));
    juniper_sstream->Start(label_map, data_manipulation, data_wrapper,
        kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
        juniper_tlm_hdr_ext);
    //int juniper_counter {0};
    void *juniper_tag {nullptr};
    bool juniper_ok {false};
    while (true) {
        //std::cout << "Juniper: " << juniper_counter << "\n";
        GPR_ASSERT(juniper_cq_->Next(&juniper_tag, &juniper_ok));
        //GPR_ASSERT(juniper_ok);
        if (juniper_ok == false) {
            spdlog::get("multi-logger")->
                warn("[JuniperFsmCtrl][grpc::CompletionQueue] "
                "unsuccessful event");
            continue;
        }
        static_cast<JuniperStream *>(juniper_tag)->Srv::JuniperStream::Start(
            label_map, data_manipulation, data_wrapper, kafka_delivery,
            kafka_producer, zmq_pusher, zmq_sock, zmq_uri, juniper_tlm_hdr_ext);
        //juniper_counter++;
    }

    if (ddm.compare("kafka") == 0) {
        kafka_producer.close();
    } else if (ddm.compare("zmq") == 0) {
        zmq_sock.close();
    }
}

void Srv::HuaweiFsmCtrl()
{
    auto tid = std::this_thread::get_id();
    std::stringstream stid;
    stid << tid;
    spdlog::get("multi-logger")->debug("Srv::HuaweiFsmCtrl() - Thread-ID: {}",
        stid.str());

    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    DataManipulation data_manipulation;
    DataWrapper data_wrapper;
    huawei_telemetry::Telemetry huawei_tlm;
    openconfig_interfaces::Interfaces oc_if;

    // Kafka producer
    KafkaDelivery kafka_delivery;
    const kafka::Properties kproperties = kafka_delivery.get_properties();
    kafka::clients::KafkaProducer kafka_producer(kproperties);

    // Zmq pusher
    ZmqPush zmq_pusher;
    const std::string zmq_uri = zmq_pusher.get_zmq_transport_uri();
    zmq::socket_t zmq_sock(zmq_pusher.get_zmq_ctx(),
        zmq::socket_type::push);
    zmq_sock.connect(zmq_uri);

    if (ddm.compare("kafka") != 0) {
        kafka_producer.~KafkaProducer();
        kafka_delivery.~KafkaDelivery();
    } else if (ddm.compare("zmq") != 0) {
        zmq_sock.~socket_t();
        zmq_pusher.~ZmqPush();
    } else {
        spdlog::get("multi-logger")->
            error("[Srv::CiscoFsmCtrl()]: Unable to set the "
            "delivery function");
        std::abort();
    }

    std::unique_ptr<Srv::HuaweiStream> huawei_sstream(
        new Srv::HuaweiStream(&huawei_service_, huawei_cq_.get()));
    huawei_sstream->Start(label_map, data_manipulation, data_wrapper,
        kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
        huawei_tlm, oc_if);
    //int huawei_counter {0};
    void *huawei_tag {nullptr};
    bool huawei_ok {false};
    while (true) {
        //std::cout << "Huawei: " << huawei_counter << "\n";
        GPR_ASSERT(huawei_cq_->Next(&huawei_tag, &huawei_ok));
        //GPR_ASSERT(huawei_ok);
        if (huawei_ok == false) {
            spdlog::get("multi-logger")->
                warn("[HuaweiFsmCtrl][grpc::CompletionQueue] "
                "unsuccessful event");
            continue;
        }
        static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Start(
            label_map, data_manipulation, data_wrapper, kafka_delivery,
            kafka_producer, zmq_pusher, zmq_sock, zmq_uri, huawei_tlm, oc_if);
        //huawei_counter++;
    }

    if (ddm.compare("kafka") == 0) {
        kafka_producer.close();
    } else if (ddm.compare("zmq") == 0) {
        zmq_sock.close();
    }
}

Srv::CiscoStream::CiscoStream(
    mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
    grpc::ServerCompletionQueue *cisco_cq) :
        cisco_service_ {cisco_service},
        cisco_cq_ {cisco_cq},
        cisco_resp {&cisco_server_ctx},
        cisco_replies_sent {0},
        kCiscoMaxReplies
            {std::stoi(main_cfg_parameters.at("replies_cisco"))},
        cisco_stream_status {START}
{
    spdlog::get("multi-logger")->debug("constructor: CiscoStream()");
}

Srv::JuniperStream::JuniperStream(
    Subscriber::AsyncService *juniper_service,
    grpc::ServerCompletionQueue *juniper_cq) :
        juniper_service_ {juniper_service},
        juniper_cq_ {juniper_cq},
        juniper_resp {&juniper_server_ctx},
        juniper_replies_sent {0},
        kJuniperMaxReplies
            {std::stoi(main_cfg_parameters.at("replies_juniper"))},
        juniper_stream_status {START}
{
    spdlog::get("multi-logger")->debug("constructor: JuniperStream()");
}

Srv::HuaweiStream::HuaweiStream(
    huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
    grpc::ServerCompletionQueue *huawei_cq) :
        huawei_service_ {huawei_service},
        huawei_cq_ {huawei_cq},
        huawei_resp {&huawei_server_ctx},
        huawei_replies_sent {0},
        kHuaweiMaxReplies
            {std::stoi(main_cfg_parameters.at("replies_huawei"))},
        huawei_stream_status {START}
{
    spdlog::get("multi-logger")->debug("constructor: HuaweiStream()");
}

void Srv::CiscoStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    DataManipulation &data_manipulation,
    DataWrapper &data_wrapper,
    KafkaDelivery &kafka_delivery,
    kafka::clients::KafkaProducer &kafka_producer,
    ZmqPush &zmq_pusher,
    zmq::socket_t &zmq_sock,
    const std::string &zmq_uri,
    cisco_telemetry::Telemetry &cisco_tlm)
{
    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    Srv::CiscoStream *cisco_sstream =
        new Srv::CiscoStream(cisco_service_, cisco_cq_);

    // Initial stream_status set to START @constructor
    if (cisco_stream_status == START) {
        cisco_service_->RequestMdtDialout(
            &cisco_server_ctx,
            &cisco_resp,
            cisco_cq_,
            cisco_cq_,
            this);
        cisco_stream_status = FLOW;
    } else if (cisco_stream_status == FLOW) {
        spdlog::get("multi-logger")->debug("[CiscoStream::Start()] "
            "new Srv::CiscoStream() {}", cisco_server_ctx.peer());
        cisco_sstream->Start(label_map, data_manipulation, data_wrapper,
            kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
            cisco_tlm);
        cisco_resp.Read(&cisco_stream, this);
        cisco_stream_status = PROCESSING;
        cisco_replies_sent++;
    } else if (cisco_stream_status == PROCESSING) {
        if (cisco_replies_sent == kCiscoMaxReplies) {
            spdlog::get("multi-logger")->debug("[CiscoStream::Start()] "
                "cisco_stream_status = END");
            cisco_stream_status = END;
            cisco_resp.Finish(grpc::Status::OK, this);
            if (cisco_sstream) {
                delete cisco_sstream;
                cisco_sstream = nullptr;
            }
        } else {
            auto tid = std::this_thread::get_id();
            std::stringstream stid;
            stid << tid;
            spdlog::get("multi-logger")->debug(
                "Srv::CiscoStream::Start() - Thread-ID: {}",
                stid.str());
            //  --- DEBUG ---
            //for (auto &e : label_map) {
            //    std::cout << e.first << " ---> "
            //    << "[" << e.second.at(0) << ","
            //    << e.second.at(1) << "]\n";
            //}
            //  --- DEBUG ---
            bool parsing_str {false};
            // From the network
            std::string stream_data_in;
            // After normalization
            std::string stream_data_in_normalization;
            // After meta-data
            std::string stream_data_out_meta;
            // After data enrichment
            std::string stream_data_out;
            const std::string _peer = cisco_server_ctx.peer();
            // select exclusively the IP addr/port from peer
            int d1 = (_peer.find_first_of(":") + 1);
            int d2 = _peer.find_last_of(":");
            const std::string peer_ip = _peer.substr(d1, (d2 - d1));
            const std::string peer_port = _peer.substr(
                (d2 + 1), ((_peer.npos - 1) - (d2 + 1)));
            // the key-word "this" is used as a unique TAG
            cisco_resp.Read(&cisco_stream, this);
            // returns true for GPB-KV & GPB, false for JSON
            // (from protobuf libs)
            parsing_str = cisco_tlm.ParseFromString(cisco_stream.data());

            stream_data_in = cisco_stream.data();

            // Handling empty data
            if (stream_data_in.empty() == true) {
                spdlog::get("multi-logger")->
                    info("[CiscoStream::Start()] {} handling empty "
                    "data", peer_ip);
            // Handling GPB-KV
            } else if (cisco_tlm.data_gpbkv().empty() == false &&
                parsing_str == true) {
                spdlog::get("multi-logger")->
                    info("[CiscoStream::Start()] {} handling GPB-KV "
                    "data", peer_ip);
                // std::string:compare returns 0 when the compared strings are
                // matching
                if (data_manipulation_cfg_parameters.at(
                    "enable_cisco_gpbkv2json").compare("true") == 0) {
                    if (data_manipulation.CiscoGpbkv2Json(cisco_tlm,
                        stream_data_in_normalization) == true) {
                        spdlog::get("multi-logger")->
                            info("[CiscoStream::Start()] {} "
                            "enable_cisco_gpbkv2json, data-normalization "
                            "successful", peer_ip);
                        // Data enrichment with label (node_id/platform_id)
                        if (data_manipulation_cfg_parameters.at(
                            "enable_label_encode_as_map").compare("true")
                                == 0 || data_manipulation_cfg_parameters.at(
                            "enable_label_encode_as_map_ptm").compare("true")
                                == 0) {
                            if (data_manipulation.MetaData(
                                    stream_data_in_normalization,
                                    peer_ip,
                                    peer_port,
                                    stream_data_out_meta) == true &&
                                data_manipulation.AppendLabelMap(
                                    label_map,
                                    peer_ip,
                                    stream_data_out_meta,
                                    stream_data_out) == true ) {
                                if (ddm.compare("kafka") == 0) {
                                    kafka_delivery.AsyncKafkaProducer(
                                        kafka_producer,
                                        peer_ip,
                                        stream_data_out);
                                } else if (ddm.compare("zmq") == 0) {
                                    data_wrapper.BuildDataWrapper (
                                        "gRPC",
                                        "json_string",
                                        main_cfg_parameters.at("writer_id"),
                                        peer_ip,
                                        peer_port,
                                        stream_data_in_normalization);
                                    zmq_pusher.ZmqPusher(
                                        data_wrapper,
                                        zmq_sock,
                                        zmq_uri);
                                }
                            }
                        } else {
                            if (data_manipulation.MetaData(
                                    stream_data_in_normalization,
                                    peer_ip,
                                    peer_port,
                                    stream_data_out_meta) == true) {
                                if (ddm.compare("kafka") == 0) {
                                    kafka_delivery.AsyncKafkaProducer(
                                        kafka_producer,
                                        peer_ip,
                                        stream_data_out_meta);
                                } else if (ddm.compare("zmq") == 0) {
                                    data_wrapper.BuildDataWrapper (
                                        "gRPC",
                                        "json_string",
                                        main_cfg_parameters.at("writer_id"),
                                        peer_ip,
                                        peer_port,
                                        stream_data_in_normalization);
                                    zmq_pusher.ZmqPusher(
                                        data_wrapper,
                                        zmq_sock,
                                        zmq_uri);
                                    }
                            }
                        }
                    } else {
                        spdlog::get("multi-logger")->
                            error("[CiscoStream::Start()] {} "
                            "enable_cisco_gpbkv2json, data-normalization "
                            "failure", peer_ip);
                    }
                } else if (data_manipulation_cfg_parameters.at(
                    "enable_cisco_message_to_json_string").compare(
                    "true") == 0) {
                    // MessageToJson is working directly on the PROTO-Obj
                    stream_data_in.clear();
                    google::protobuf::util::JsonPrintOptions opt;
                    opt.add_whitespace = false;
                    google::protobuf::util::MessageToJsonString(
                        cisco_tlm,
                        &stream_data_in,
                        opt);
                    // Data enrichment with label (node_id/platform_id)
                    if (data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map").compare("true") == 0 ||
                        data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map_ptm").compare("true")
                            == 0) {
                        if (data_manipulation.MetaData(
                                stream_data_in,
                                peer_ip,
                                peer_port,
                                stream_data_out_meta) == true &&
                            data_manipulation.AppendLabelMap(
                                label_map,
                                peer_ip,
                                stream_data_out_meta,
                                stream_data_out) == true) {
                            if (ddm.compare("kafka") == 0) {
                                kafka_delivery.AsyncKafkaProducer(
                                    kafka_producer,
                                    peer_ip,
                                    stream_data_out);
                            } else if (ddm.compare("zmq") == 0) {
                                data_wrapper.BuildDataWrapper (
                                    "gRPC",
                                    "json_string",
                                    main_cfg_parameters.at("writer_id"),
                                    peer_ip,
                                    peer_port,
                                    stream_data_in);
                                zmq_pusher.ZmqPusher(
                                    data_wrapper,
                                    zmq_sock,
                                    zmq_uri);
                                }
                        }
                    } else {
                        if (data_manipulation.MetaData(
                                stream_data_in,
                                peer_ip,
                                peer_port,
                                stream_data_out_meta) == true) {
                            if (ddm.compare("kafka") == 0) {
                                kafka_delivery.AsyncKafkaProducer(
                                    kafka_producer,
                                    peer_ip,
                                    stream_data_out_meta);
                            } else if (ddm.compare("zmq") == 0) {
                                data_wrapper.BuildDataWrapper (
                                    "gRPC",
                                    "json_string",
                                    main_cfg_parameters.at("writer_id"),
                                    peer_ip,
                                    peer_port,
                                    stream_data_in);
                                zmq_pusher.ZmqPusher(
                                    data_wrapper,
                                    zmq_sock,
                                    zmq_uri);
                                }
                        }
                    }
                } else {
                    // Use Case: both data manipulation funcs set to false:
                    // TBD: at the meoment simply alert
                    spdlog::get("multi-logger")->
                        error("[CiscoStream::Start()] {} "
                        "general data-manipulation failure "
                        "conflicting manipulation functions", peer_ip);
                }

            // Handling GPB
            } else if (cisco_tlm.has_data_gpb() == true &&
                parsing_str == true) {
                spdlog::get("multi-logger")->
                    info("[CiscoStream::Start()] {} handling GPB "
                    "data", peer_ip);
                // Data enrichment with label (node_id/platform_id)
                if (data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map").compare("true") == 0 ||
                    data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map_ptm").compare("true") == 0) {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true &&
                        data_manipulation.AppendLabelMap(
                            label_map,
                            peer_ip,
                            stream_data_out_meta,
                            stream_data_out) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_uri);
                            }
                    }
                } else {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out_meta);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_uri);
                            }
                    }
                }
            // Handling JSON string
            } else if (parsing_str == false) {
                spdlog::get("multi-logger")->
                    info("[CiscoStream::Start()] {} handling JSON "
                    "data", peer_ip);
                // Data enrichment with label (node_id/platform_id)
                if (data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map").compare("true") == 0 ||
                    data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map_ptm").compare("true") == 0) {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true &&
                        data_manipulation.AppendLabelMap(
                            label_map,
                            peer_ip,
                            stream_data_out_meta,
                            stream_data_out) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_uri);
                        }
                    }
                } else {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out_meta);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_uri);
                        }
                    }
                }
            }
            cisco_stream_status = PROCESSING;
            cisco_replies_sent++;
            if (cisco_sstream) {
                delete cisco_sstream;
                cisco_sstream = nullptr;
            }
        }
    } else {
        spdlog::get("multi-logger")->debug("[CiscoStream::Start()] "
            "GPR_ASSERT(cisco_stream_status == END)");
        GPR_ASSERT(cisco_stream_status == END);
        delete this;
    }
}

void Srv::JuniperStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    DataManipulation &data_manipulation,
    DataWrapper &data_wrapper,
    KafkaDelivery &kafka_delivery,
    kafka::clients::KafkaProducer &kafka_producer,
    ZmqPush &zmq_pusher,
    zmq::socket_t &zmq_sock,
    const std::string &zmq_uri,
    GnmiJuniperTelemetryHeaderExtension &juniper_tlm_hdr_ext)
{
    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    Srv::JuniperStream *juniper_sstream =
        new Srv::JuniperStream(juniper_service_, juniper_cq_);

    // Initial stream_status set to START @constructor
    if (juniper_stream_status == START) {
        juniper_service_->RequestDialOutSubscriber(
            &juniper_server_ctx,
            &juniper_resp,
            juniper_cq_,
            juniper_cq_,
            this);
        juniper_stream_status = FLOW;
    } else if (juniper_stream_status == FLOW) {
        spdlog::get("multi-logger")->debug("[JuniperStream::Start()] "
            "new Srv::JuniperStream() {}", juniper_server_ctx.peer());
        juniper_sstream->Start(label_map, data_manipulation, data_wrapper,
            kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
            juniper_tlm_hdr_ext);
        juniper_resp.Read(&juniper_stream, this);
        juniper_stream_status = PROCESSING;
        juniper_replies_sent++;
    } else if (juniper_stream_status == PROCESSING) {
        if (juniper_replies_sent == kJuniperMaxReplies) {
            spdlog::get("multi-logger")->debug("[JuniperStream::Start()] "
                "juniper_stream_status = END");
            juniper_stream_status = END;
            juniper_resp.Finish(grpc::Status::OK, this);
            if (juniper_sstream) {
                delete juniper_sstream;
                juniper_sstream = nullptr;
            }
        } else {
            auto tid = std::this_thread::get_id();
            std::stringstream stid;
            stid << tid;
            spdlog::get("multi-logger")->debug(
                "Srv::JuniperStream::Start() - Thread-ID: {}",
                stid.str());
            // From the network
            std::string stream_data_in;
            // After meta-data
            std::string stream_data_out_meta;
            // After data enrichment
            std::string stream_data_out;
            std::string json_str_out;
            const std::string _peer = juniper_server_ctx.peer();
            // select exclusively the IP addr/port from peer
            int d1 = (_peer.find_first_of(":") + 1);
            int d2 = _peer.find_last_of(":");
            const std::string peer_ip = _peer.substr(d1, (d2 - d1));
            const std::string peer_port = _peer.substr(
                (d2 + 1), ((_peer.npos - 1) - (d2 + 1)));

            Json::Value root;

            // the key-word "this" is used as a unique TAG
            juniper_resp.Read(&juniper_stream, this);

            if (data_manipulation.JuniperExtension(juniper_stream,
                juniper_tlm_hdr_ext, root) == true &&
                data_manipulation.JuniperUpdate(juniper_stream, json_str_out,
                    root) == true) {
                    spdlog::get("multi-logger")->
                        info("[JuniperStream::Start()] {} "
                        "JuniperExtension, parsing successful", peer_ip);
            } else {
                    spdlog::get("multi-logger")->
                        error("[JuniperStream::Start()] {} "
                        "JuniperExtension, parsing failure", peer_ip);
            }

            stream_data_in = json_str_out;

            // Data enrichment with label (node_id/platform_id)
            if (data_manipulation_cfg_parameters.at(
                "enable_label_encode_as_map").compare("true") == 0 ||
                data_manipulation_cfg_parameters.at(
                "enable_label_encode_as_map_ptm").compare("true") == 0) {
                if (data_manipulation.MetaData(
                        stream_data_in,
                        peer_ip,
                        peer_port,
                        stream_data_out_meta) == true &&
                    data_manipulation.AppendLabelMap(
                        label_map,
                        peer_ip,
                        stream_data_out_meta,
                        stream_data_out) == true) {
                    if (ddm.compare("kafka") == 0) {
                        kafka_delivery.AsyncKafkaProducer(
                            kafka_producer,
                            peer_ip,
                            stream_data_out);
                    } else if (ddm.compare("zmq") == 0) {
                        data_wrapper.BuildDataWrapper (
                            "gRPC",
                            "json_string",
                            main_cfg_parameters.at("writer_id"),
                            peer_ip,
                            peer_port,
                            stream_data_in);
                        zmq_pusher.ZmqPusher(
                            data_wrapper,
                            zmq_sock,
                            zmq_pusher.get_zmq_transport_uri());
                    }
                }
            } else {
                if (data_manipulation.MetaData(
                        stream_data_in,
                        peer_ip,
                        peer_port,
                        stream_data_out_meta) == true) {
                    if (ddm.compare("kafka") == 0) {
                        kafka_delivery.AsyncKafkaProducer(
                            kafka_producer,
                            peer_ip,
                            stream_data_out_meta);
                    } else if (ddm.compare("zmq") == 0) {
                        data_wrapper.BuildDataWrapper (
                            "gRPC",
                            "json_string",
                            main_cfg_parameters.at("writer_id"),
                            peer_ip,
                            peer_port,
                            stream_data_in);
                        zmq_pusher.ZmqPusher(
                            data_wrapper,
                            zmq_sock,
                            zmq_pusher.get_zmq_transport_uri());
                    }
                }
            }
            juniper_stream_status = PROCESSING;
            juniper_replies_sent++;
            if (juniper_sstream) {
                delete juniper_sstream;
                juniper_sstream = nullptr;
            }
        }
    } else {
        spdlog::get("multi-logger")->debug("[JuniperStream::Start()] "
            "GPR_ASSERT(juniper_stream_status == END)");
        GPR_ASSERT(juniper_stream_status == END);
        delete this;
    }
}

void Srv::HuaweiStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map,
    DataManipulation &data_manipulation,
    DataWrapper &data_wrapper,
    KafkaDelivery &kafka_delivery,
    kafka::clients::KafkaProducer &kafka_producer,
    ZmqPush &zmq_pusher,
    zmq::socket_t &zmq_sock,
    const std::string &zmq_uri,
    huawei_telemetry::Telemetry &huawei_tlm,
    openconfig_interfaces::Interfaces &oc_if)
{
    const std::string ddm = main_cfg_parameters.at("data_delivery_method");

    Srv::HuaweiStream *huawei_sstream =
        new Srv::HuaweiStream(huawei_service_, huawei_cq_);

    // Initial stream_status set to START @constructor
    if (huawei_stream_status == START) {
        huawei_service_->RequestdataPublish(
            &huawei_server_ctx,
            &huawei_resp,
            huawei_cq_,
            huawei_cq_,
            this);
        huawei_stream_status = FLOW;
    } else if (huawei_stream_status == FLOW) {
        spdlog::get("multi-logger")->debug("[HuaweiStream::Start()] "
            "new Srv::HuaweiStream()");
        huawei_sstream->Start(label_map, data_manipulation, data_wrapper,
            kafka_delivery, kafka_producer, zmq_pusher, zmq_sock, zmq_uri,
            huawei_tlm, oc_if);
        huawei_resp.Read(&huawei_stream, this);
        huawei_stream_status = PROCESSING;
        huawei_replies_sent++;
    } else if (huawei_stream_status == PROCESSING) {
        if (huawei_replies_sent == kHuaweiMaxReplies) {
            spdlog::get("multi-logger")->debug("[HuaweiStream::Start()] "
                "huawei_stream_status = END");
            huawei_stream_status = END;
            huawei_resp.Finish(grpc::Status::OK, this);
            if (huawei_sstream) {
                 delete huawei_sstream;
                 huawei_sstream = nullptr;
            }
        } else {
            auto tid = std::this_thread::get_id();
            std::stringstream stid;
            stid << tid;
            spdlog::get("multi-logger")->debug(
                "Srv::HuaweiStream::Start() - Thread-ID: {}",
                stid.str());
            bool parsing_str {false};
            // From the network
            std::string stream_data_in;
            // After meta-data
            std::string stream_data_out_meta;
            // Afetr data enrichment
            std::string stream_data_out;
            std::string json_str_out;
            const std::string _peer = huawei_server_ctx.peer();
            // select exclusively the IP addr/port from peer
            int d1 = (_peer.find_first_of(":") + 1);
            int d2 = _peer.find_last_of(":");
            const std::string peer_ip = _peer.substr(d1, (d2 - d1));
            const std::string peer_port = _peer.substr(
                (d2 + 1), ((_peer.npos - 1) - (d2 + 1)));

            huawei_resp.Read(&huawei_stream, this);
            parsing_str = huawei_tlm.ParseFromString(huawei_stream.data());

            stream_data_in = huawei_stream.data();

            // Handling empty data
            if (stream_data_in.empty() == true) {
                spdlog::get("multi-logger")->
                    info("[HuaweiStream::Start()] {} handling empty "
                    "data", peer_ip);
            }

            // Handling GPB
            else {
                // Handling OpenConfig interfaces
                if (huawei_tlm.has_data_gpb() == true &&
                    parsing_str == true &&
                    huawei_tlm.proto_path().compare(
                        "openconfig_interfaces.Interfaces") == 0) {
                    spdlog::get("multi-logger")->
                        info("[HuaweiStream::Start()] {} handling "
                        "GPB data", peer_ip);

                    if (data_manipulation.HuaweiGpbOpenconfigInterface(
                        huawei_tlm, oc_if, json_str_out) == true) {
                            spdlog::get("multi-logger")->
                                info("[HuaweiStream::Start()] {} "
                                "HuaweiGpbOpenconfigInterface, "
                                "parsing successful", peer_ip);
                    } else {
                        spdlog::get("multi-logger")->
                            error("[HuaweiStream::Start()] {}"
                            "HuaweiGpbOpenconfigInterface, "
                            "parsing failure", peer_ip);
                    }

                    // Data enrichment with label (node_id/platform_id)
                    stream_data_in = json_str_out;
                    if (data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map").compare("true") == 0 ||
                        data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map_ptm").compare("true")
                            == 0) {
                        if (data_manipulation.MetaData(
                                stream_data_in,
                                peer_ip,
                                peer_port,
                                stream_data_out_meta) == true &&
                            data_manipulation.AppendLabelMap(
                                label_map,
                                peer_ip,
                                stream_data_out_meta,
                                stream_data_out) == true) {
                            if (ddm.compare("kafka") == 0) {
                                kafka_delivery.AsyncKafkaProducer(
                                    kafka_producer,
                                    peer_ip,
                                    stream_data_out);
                            } else if (ddm.compare("zmq") == 0) {
                                data_wrapper.BuildDataWrapper (
                                    "gRPC",
                                    "json_string",
                                    main_cfg_parameters.at("writer_id"),
                                    peer_ip,
                                    peer_port,
                                    stream_data_in);
                                zmq_pusher.ZmqPusher(
                                    data_wrapper,
                                    zmq_sock,
                                    zmq_pusher.get_zmq_transport_uri());
                            }
                        }
                    } else {
                        if (data_manipulation.MetaData(
                                stream_data_in,
                                peer_ip,
                                peer_port,
                                stream_data_out_meta) == true) {
                            if (ddm.compare("kafka") == 0) {
                                kafka_delivery.AsyncKafkaProducer(
                                    kafka_producer,
                                    peer_ip,
                                    stream_data_out_meta);
                            } else if (ddm.compare("zmq") == 0) {
                                data_wrapper.BuildDataWrapper (
                                    "gRPC",
                                    "json_string",
                                    main_cfg_parameters.at("writer_id"),
                                    peer_ip,
                                    peer_port,
                                    stream_data_in);
                                zmq_pusher.ZmqPusher(
                                    data_wrapper,
                                    zmq_sock,
                                    zmq_pusher.get_zmq_transport_uri());
                            }
                        }
                    }
                }
            }

            stream_data_in = huawei_stream.data_json();

            // Handling empty data_json
            if (stream_data_in.empty() == true) {
                spdlog::get("multi-logger")->
                    info("[HuaweiStream::Start()] {} handling empty "
                    "data_json", peer_ip);
            }
            // Handling JSON string
            else {
                // ---
                spdlog::get("multi-logger")->
                    info("[HuaweiStream::Start()] {} handling JSON "
                    "data_json", peer_ip);

                // Data enrichment with label (node_id/platform_id)
                if (data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map").compare("true") == 0 ||
                    data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map_ptm").compare("true") == 0) {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true &&
                        data_manipulation.AppendLabelMap(
                            label_map,
                            peer_ip,
                            stream_data_out_meta,
                            stream_data_out) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_pusher.get_zmq_transport_uri());
                        }
                    }
                } else {
                    if (data_manipulation.MetaData(
                            stream_data_in,
                            peer_ip,
                            peer_port,
                            stream_data_out_meta) == true) {
                        if (ddm.compare("kafka") == 0) {
                            kafka_delivery.AsyncKafkaProducer(
                                kafka_producer,
                                peer_ip,
                                stream_data_out_meta);
                        } else if (ddm.compare("zmq") == 0) {
                            data_wrapper.BuildDataWrapper (
                                "gRPC",
                                "json_string",
                                main_cfg_parameters.at("writer_id"),
                                peer_ip,
                                peer_port,
                                stream_data_in);
                            zmq_pusher.ZmqPusher(
                                data_wrapper,
                                zmq_sock,
                                zmq_pusher.get_zmq_transport_uri());
                        }
                    }
                }
            }
            huawei_stream_status = PROCESSING;
            huawei_replies_sent++;
            if (huawei_sstream) {
                 delete huawei_sstream;
                 huawei_sstream = nullptr;
            }
        }
    } else {
        spdlog::get("multi-logger")->debug("[HuaweiStream::Start()] "
            "GPR_ASSERT(huawei_stream_status == END)");
        GPR_ASSERT(huawei_stream_status == END);
        delete this;
    }
}

