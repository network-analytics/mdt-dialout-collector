// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "core/mdt_dialout_core.h"


// Global visibility to be able to signal the refresh --> CSV from main
std::unordered_map<std::string, std::vector<std::string>> label_map;

bool CustomSocketMutator::bindtodevice_socket_mutator(int fd)
{
    int type;
    socklen_t len = sizeof(type);

    std::string iface = main_cfg_parameters.at("iface");

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
        //std::cout << "Issues with getting the iface type ..." << "\n";
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
        iface.c_str(), strlen(iface.c_str())) != 0) {
        //std::cout << "Issues with iface binding for ..." << "\n";
    }

    return true;
}

bool custom_socket_mutator_fd(int fd, grpc_socket_mutator *mutator0) {
    CustomSocketMutator *csm = (CustomSocketMutator *)mutator0;
    return csm->bindtodevice_socket_mutator(fd);
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

const grpc_socket_mutator_vtable
    custom_socket_mutator_vtable = grpc_socket_mutator_vtable{
        custom_socket_mutator_fd,
        custom_socket_compare,
        custom_socket_destroy,
        nullptr};

void ServerBuilderOptionImpl::UpdateArguments(
    grpc::ChannelArguments *custom_args) {
        CustomSocketMutator *csm_ = new CustomSocketMutator();
        custom_args->SetSocketMutator(csm_);
}

CustomSocketMutator::CustomSocketMutator() {
    std::cout << "CustomSocketMutator()\n";
    grpc_socket_mutator_init(this, &custom_socket_mutator_vtable);
}

void Srv::CiscoBind(std::string cisco_srv_socket)
{
    grpc::ServerBuilder cisco_builder;
    cisco_builder.RegisterService(&cisco_service_);
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        csbo(new ServerBuilderOptionImpl());
    cisco_builder.SetOption(std::move(csbo));
    // --- Required for socket manipulation ---
    cisco_builder.AddListeningPort(cisco_srv_socket,
        grpc::InsecureServerCredentials());
    cisco_cq_ = cisco_builder.AddCompletionQueue();
    cisco_server_ = cisco_builder.BuildAndStart();

    Srv::CiscoFsmCtrl();
}

void Srv::JuniperBind(std::string juniper_srv_socket)
{
    grpc::ServerBuilder juniper_builder;
    juniper_builder.RegisterService(&juniper_service_);
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        jsbo(new ServerBuilderOptionImpl());
    juniper_builder.SetOption(std::move(jsbo));
    // --- Required for socket manipulation ---
    juniper_builder.AddListeningPort(juniper_srv_socket,
        grpc::InsecureServerCredentials());
    juniper_cq_ = juniper_builder.AddCompletionQueue();
    juniper_server_ = juniper_builder.BuildAndStart();

    Srv::JuniperFsmCtrl();
}

void Srv::HuaweiBind(std::string huawei_srv_socket)
{
    grpc::ServerBuilder huawei_builder;
    huawei_builder.RegisterService(&huawei_service_);
    // --- Required for socket manipulation ---
    std::unique_ptr<ServerBuilderOptionImpl>
        hsbo(new ServerBuilderOptionImpl());
    huawei_builder.SetOption(std::move(hsbo));
    // --- Required for socket manipulation ---
    huawei_builder.AddListeningPort(huawei_srv_socket,
        grpc::InsecureServerCredentials());
    huawei_cq_ = huawei_builder.AddCompletionQueue();
    huawei_server_ = huawei_builder.BuildAndStart();

    Srv::HuaweiFsmCtrl();
}

void Srv::CiscoFsmCtrl()
{
    new Srv::CiscoStream(&cisco_service_, cisco_cq_.get());
    //int cisco_counter {0};
    void *cisco_tag {nullptr};
    bool cisco_ok {false};
    while (true) {
        //std::cout << "Cisco: " << cisco_counter << "\n";
        GPR_ASSERT(cisco_cq_->Next(&cisco_tag, &cisco_ok));
        //GPR_ASSERT(cisco_ok);
        if (cisco_ok == false) {
            std::cout << "WARN - Cisco CQ failed\n";
            continue;
        }
        static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Start(
            label_map);
        //cisco_counter++;
    }
}

void Srv::JuniperFsmCtrl()
{
    new Srv::JuniperStream(&juniper_service_, juniper_cq_.get());
    //int juniper_counter {0};
    void *juniper_tag {nullptr};
    bool juniper_ok {false};
    while (true) {
        //std::cout << "Juniper: " << juniper_counter << "\n";
        GPR_ASSERT(juniper_cq_->Next(&juniper_tag, &juniper_ok));
        //GPR_ASSERT(juniper_ok);
        if (juniper_ok == false) {
            std::cout << "WARN - Juniper CQ failed\n";
            continue;
        }
        static_cast<JuniperStream *>(juniper_tag)->Srv::JuniperStream::Start(
            label_map);
        //juniper_counter++;
    }
}

void Srv::HuaweiFsmCtrl()
{
    new Srv::HuaweiStream(&huawei_service_, huawei_cq_.get());
    //int huawei_counter {0};
    void *huawei_tag {nullptr};
    bool huawei_ok {false};
    while (true) {
        //std::cout << "Huawei: " << huawei_counter << "\n";
        GPR_ASSERT(huawei_cq_->Next(&huawei_tag, &huawei_ok));
        //GPR_ASSERT(huawei_ok);
        if (huawei_ok == false) {
            std::cout << "WARN - Huawei CQ failed\n";
            continue;
        }
        static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Start(
            label_map);
        //huawei_counter++;
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
    std::cout << "CiscoStream()\n";
    Srv::CiscoStream::Start(label_map);
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
    std::cout << "JuniperStream()\n";
    Srv::JuniperStream::Start(label_map);
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
    std::cout << "HuaweiStream()\n";
    Srv::HuaweiStream::Start(label_map);
}

// --- DataManipulation, Datadelivery & Decoding
std::unique_ptr<DataManipulation> data_manipulation(
    new DataManipulation());
std::unique_ptr<DataDelivery> data_delivery(
    new DataDelivery());
std::unique_ptr<cisco_telemetry::Telemetry> cisco_tlm(
    new cisco_telemetry::Telemetry());
std::unique_ptr<GnmiJuniperTelemetryHeaderExtension> juniper_tlm_hdr_ext(
    new GnmiJuniperTelemetryHeaderExtension());
std::unique_ptr<huawei_telemetry::Telemetry> huawei_tlm(
    new huawei_telemetry::Telemetry());
std::unique_ptr<openconfig_interfaces::Interfaces> oc_if(
    new openconfig_interfaces::Interfaces());
// --- DataManipulation, Datadelivery & Decoding

void Srv::CiscoStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map)
{
    // Initial stream_status set to START
    if (cisco_stream_status == START) {
        cisco_service_->RequestMdtDialout(
            &cisco_server_ctx,
            &cisco_resp,
            cisco_cq_,
            cisco_cq_,
            this);
        cisco_stream_status = FLOW;
    } else if (cisco_stream_status == FLOW) {
        std::cout << "new Srv::CiscoStream()\n";
        new Srv::CiscoStream(cisco_service_, cisco_cq_);
        cisco_resp.Read(&cisco_stream, this);
        cisco_stream_status = PROCESSING;
        cisco_replies_sent++;
    } else if (cisco_stream_status == PROCESSING) {
        if (cisco_replies_sent == kCiscoMaxReplies) {
            std::cout << "cisco_stream_status = END\n";
            cisco_stream_status = END;
            cisco_resp.Finish(grpc::Status::OK, this);
        } else {
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
            // After data enrichment
            std::string stream_data_out;
            const std::string peer = cisco_server_ctx.peer();
            // the key-word "this" is used as a unique TAG
            cisco_resp.Read(&cisco_stream, this);
            // returns true for GPB-KV & GPB, false for JSON
            // (from protobuf libs)
            parsing_str = cisco_tlm->ParseFromString(cisco_stream.data());

            stream_data_in = cisco_stream.data();

            // Handling empty data
            if (stream_data_in.empty() == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " CISCO Handling empty data: "
                    << type_info << "\n";
               // ---

            // Handling GPB-KV
            } else if (cisco_tlm->data_gpbkv().empty() == false &&
                parsing_str == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " CISCO Handling GPB-KV: " << type_info
                    << "\n";
                // ---

                // std::string:compare returns 0 when the compared strings are
                // matching
                if (data_manipulation_cfg_parameters.at(
                    "enable_cisco_gpbkv2json").compare("true") == 0) {
                    if (data_manipulation->CiscoGpbkv2Json(cisco_tlm,
                        stream_data_in) == true) {
                        std::cout << "Cisco data-normalization successful\n";
                    } else {
                        std::cout << "Cisco data-normalization unsuccessful\n";
                    }
                } else if (data_manipulation_cfg_parameters.at(
                    "enable_cisco_message_to_json_string").compare(
                    "true") == 0) {
                    // MessageToJson is working directly on the PROTO-Obj
                    stream_data_in.clear();
                    google::protobuf::util::JsonPrintOptions opt;
                    opt.add_whitespace = true;
                    google::protobuf::util::MessageToJsonString(
                        *cisco_tlm,
                        &stream_data_in,
                        opt);
                    // Data enrichment with label (node_id/platform_id)
                    if (data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map").compare("true") == 0) {
                        if (data_manipulation->AppendLabelMap(
                            label_map,
                            peer,
                            stream_data_in,
                            stream_data_out) == true) {
                            data_delivery->AsyncKafkaProducer(
                                stream_data_out);
                        }
                    } else {
                        stream_data_out = stream_data_in;
                        data_delivery->AsyncKafkaProducer(stream_data_out);
                    }
                } else {
                    // Use Case: both data manipulation funcs set to false:
                    // TBD: at the meoment simply send binary format to stdout
                    std::cout << stream_data_in << "\n";
                }

            // Handling GPB
            } else if (cisco_tlm->has_data_gpb() == true &&
                parsing_str == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " CISCO Handling GPB: " << type_info
                    << "\n";
                // ---

                // TBD

            // Handling JSON string
            } else if (parsing_str == false) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " CISCO Handling JSON string: "
                    << type_info << "\n";
                // ---

               // Data enrichment with label (node_id/platform_id)
                if (data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map").compare("true") == 0) {
                    if (data_manipulation->AppendLabelMap(
                        label_map,
                        peer,
                        stream_data_in,
                        stream_data_out) == true) {
                        data_delivery->AsyncKafkaProducer(stream_data_out);
                    }
                } else {
                    stream_data_out = stream_data_in;
                    data_delivery->AsyncKafkaProducer(stream_data_out);
                }
            }
            cisco_stream_status = PROCESSING;
            cisco_replies_sent++;
        }
    } else {
        std::cout << "GPR_ASSERT(cisco_stream_status == END)\n";
        GPR_ASSERT(cisco_stream_status == END);
        delete this;
    }
}

void Srv::JuniperStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map)
{
    // Initial stream_status set to START
    if (juniper_stream_status == START) {
        juniper_service_->RequestDialOutSubscriber(
            &juniper_server_ctx,
            &juniper_resp,
            juniper_cq_,
            juniper_cq_,
            this);
        juniper_stream_status = FLOW;
    } else if (juniper_stream_status == FLOW) {
        std::cout << "new Srv::JuniperStream()\n";
        new Srv::JuniperStream(juniper_service_, juniper_cq_);
        juniper_resp.Read(&juniper_stream, this);
        juniper_stream_status = PROCESSING;
        juniper_replies_sent++;
    } else if (juniper_stream_status == PROCESSING) {
        if (juniper_replies_sent == kJuniperMaxReplies) {
            std::cout << "juniper_stream_status = END\n";
            juniper_stream_status = END;
            juniper_resp.Finish(grpc::Status::OK, this);
        } else {
            // From the network
            std::string stream_data_in;
            // After data enrichment
            std::string stream_data_out;
            std::string json_str_out;
            const std::string peer = juniper_server_ctx.peer();
            Json::Value root;

            // the key-word "this" is used as a unique TAG
            juniper_resp.Read(&juniper_stream, this);

            if (data_manipulation->JuniperExtension(juniper_stream,
                juniper_tlm_hdr_ext, root) == true &&
                data_manipulation->JuniperUpdate(juniper_stream, json_str_out,
                    root) == true) {
                    // to be properly logged
                    std::cout << peer
                        << " Juniper ext parsing successful" << "\n";
            } else {
                    // to be properly logged
                    std::cout << peer
                        << " Juniper ext parsing unsuccessful" << "\n";
            }

            // Data enrichment with label (node_id/platform_id)
            stream_data_in = json_str_out;
            // std::string:compare returns 0 when the compared strings are
            // matching
            if (data_manipulation_cfg_parameters.at(
                "enable_label_encode_as_map").compare("true") == 0) {
                if (data_manipulation->AppendLabelMap(
                    label_map,
                    peer,
                    stream_data_in,
                    stream_data_out) == true) {
                    data_delivery->AsyncKafkaProducer(stream_data_out);
                }
            } else {
                stream_data_out = json_str_out;
                data_delivery->AsyncKafkaProducer(stream_data_out);
            }

            juniper_stream_status = PROCESSING;
            juniper_replies_sent++;
        }
    } else {
        std::cout << "GPR_ASSERT(juniper_stream_status == END)\n";
        GPR_ASSERT(juniper_stream_status == END);
        delete this;
    }
}

void Srv::HuaweiStream::Start(
    std::unordered_map<std::string,std::vector<std::string>> &label_map)
{
    if (huawei_stream_status == START) {
        huawei_service_->RequestdataPublish(
            &huawei_server_ctx,
            &huawei_resp,
            huawei_cq_,
            huawei_cq_,
            this);
        huawei_stream_status = FLOW;
    } else if (huawei_stream_status == FLOW) {
        std::cout << "new Srv::HuaweiStream()\n";
        new Srv::HuaweiStream(huawei_service_, huawei_cq_);
        huawei_resp.Read(&huawei_stream, this);
        huawei_stream_status = PROCESSING;
        huawei_replies_sent++;
    } else if (huawei_stream_status == PROCESSING) {
        if (huawei_replies_sent == kHuaweiMaxReplies) {
            std::cout << "huawei_stream_status = END\n";
            huawei_stream_status = END;
            huawei_resp.Finish(grpc::Status::OK, this);
        } else {
            bool parsing_str {false};
            // From the network
            std::string stream_data_in;
            // Afetr data enrichment
            std::string stream_data_out;
            std::string json_str_out;
            const std::string peer = huawei_server_ctx.peer();

            huawei_resp.Read(&huawei_stream, this);
            parsing_str = huawei_tlm->ParseFromString(huawei_stream.data());

            stream_data_in = huawei_stream.data();

            // Handling empty data
            if (stream_data_in.empty() == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " HUAWEI Handling empty data: "
                    << type_info << "\n";
                // ---
            }

            // Handling GPB
            else {
                // Handling OpenConfig interfaces
                if (huawei_tlm->has_data_gpb() == true &&
                    parsing_str == true &&
                    // std::string:compare returns 0 when the compared strings
                    // are matching
                    huawei_tlm->proto_path().compare(
                        "openconfig_interfaces.Interfaces") == 0) {
                    // ---
                    auto type_info = typeid(stream_data_in).name();
                    std::cout << peer << " HUAWEI Handling GPB: " << type_info
                        << "\n";

                    if (data_manipulation->HuaweiGpbOpenconfigInterface(
                        huawei_tlm, oc_if, json_str_out) == true) {
                            // to be properly logged
                            std::cout << peer
                                << " huawei oc-if parsing succesful\n";
                    } else {
                            // to be properly logged
                            std::cout << peer
                                << " huawei oc-if parsing unsuccesful\n";
                    }

                    // Data enrichment with label (node_id/platform_id)
                    stream_data_in = json_str_out;
                    if (data_manipulation_cfg_parameters.at(
                        "enable_label_encode_as_map").compare("true") == 0) {
                        if (data_manipulation->AppendLabelMap(
                            label_map,
                            peer,
                            stream_data_in,
                            stream_data_out) == true) {
                            data_delivery->
                                AsyncKafkaProducer(stream_data_out);
                        }
                    } else {
                        stream_data_out = json_str_out;
                        data_delivery->AsyncKafkaProducer(stream_data_out);
                    }
                }
            }

            stream_data_in = huawei_stream.data_json();

            // Handling empty data_json
            if (stream_data_in.empty() == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " HUAWEI Handling empty data_json: "
                    << type_info << "\n";
                // ---
            }
            // Handling JSON string
            else {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " HUAWEI Handling JSON string: "
                    << type_info << "\n";
                // ---

                // Data enrichment with label (node_id/platform_id)
                if (data_manipulation_cfg_parameters.at(
                    "enable_label_encode_as_map").compare("true") == 0) {
                    if (data_manipulation->AppendLabelMap(
                        label_map,
                        peer,
                        stream_data_in,
                        stream_data_out) == true) {
                        data_delivery->AsyncKafkaProducer(stream_data_out);
                    }
                } else {
                    stream_data_out = stream_data_in;
                    data_delivery->AsyncKafkaProducer(stream_data_out);
                }
            }

            huawei_stream_status = PROCESSING;
            huawei_replies_sent++;
        }
    } else {
        std::cout << "GPR_ASSERT(huawei_stream_status == END)\n";
        GPR_ASSERT(huawei_stream_status == END);
        delete this;
    }
}

