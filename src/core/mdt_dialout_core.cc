#include "json/value.h"
#include <cstddef>
#include <iostream>
#include <typeinfo>
#include <grpcpp/grpcpp.h>
#include "grpc/socket_mutator.h"
#include <grpcpp/support/channel_arguments.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpcpp/impl/server_builder_plugin.h>
#include <json/json.h>
#include "core/mdt_dialout_core.h"
#include "cisco_dialout.grpc.pb.h"
#include "cisco_telemetry.pb.h"
#include "juniper_dialout.grpc.pb.h"
#include "juniper_telemetry.pb.h"
#include "juniper_gnmi.pb.h"
#include "juniper_gnmi_ext.pb.h"
#include "juniper_telemetry_header.pb.h"
#include "juniper_telemetry_header_extension.pb.h"
#include "huawei_dialout.grpc.pb.h"
#include "huawei_telemetry.pb.h"
#include <google/protobuf/util/json_util.h>
#include <sys/socket.h>
#include "cfg_handler.h"
#include "dataManipulation/data_manipulation.h"
#include "dataDelivery/data_delivery.h"
#include "openconfig_interfaces.pb.h"


bool CustomSocketMutator::bindtodevice_socket_mutator(int fd)
{
    int type;
    socklen_t len = sizeof(type);

    // --- Required for config parameters ---
    std::unique_ptr<MainCfgHandler> main_cfg_handler(new MainCfgHandler());
    std::string iface = main_cfg_handler->get_iface();
    // --- Required for config parameters ---

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
        //std::cout << "Issues with getting the iface type ..." << std::endl;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                                iface.c_str(), strlen(iface.c_str())) != 0) {
        //std::cout << "Issues with iface binding for ..." << std::endl;
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
    grpc_socket_mutator_init(this, &custom_socket_mutator_vtable);
}

Srv::~Srv()
{
    cisco_server_->grpc::ServerInterface::Shutdown();
    juniper_server_->grpc::ServerInterface::Shutdown();
    huawei_server_->grpc::ServerInterface::Shutdown();
    cisco_cq_->grpc::ServerCompletionQueue::Shutdown();
    juniper_cq_->grpc::ServerCompletionQueue::Shutdown();
    huawei_cq_->grpc::ServerCompletionQueue::Shutdown();
}

void Srv::CiscoBind(std::string cisco_srv_socket)
{
    grpc::ServerBuilder cisco_builder;
    cisco_builder.RegisterService(&cisco_service_);
    std::unique_ptr<ServerBuilderOptionImpl>
                                        csbo(new ServerBuilderOptionImpl());
    cisco_builder.SetOption(std::move(csbo));
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
    std::unique_ptr<ServerBuilderOptionImpl>
                                        jsbo(new ServerBuilderOptionImpl());
    juniper_builder.SetOption(std::move(jsbo));
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
    std::unique_ptr<ServerBuilderOptionImpl>
                                        hsbo(new ServerBuilderOptionImpl());
    huawei_builder.SetOption(std::move(hsbo));
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
        //std::cout << "Cisco: " << cisco_counter << std::endl;
        GPR_ASSERT(cisco_cq_->Next(&cisco_tag, &cisco_ok));
        //GPR_ASSERT(cisco_ok);
        if (!cisco_ok) {
            std::cout << "WARN - Cisco CQ failed\n";
            continue;
        }
        static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Start();
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
        //std::cout << "Juniper: " << juniper_counter << std::endl;
        GPR_ASSERT(juniper_cq_->Next(&juniper_tag, &juniper_ok));
        //GPR_ASSERT(juniper_ok);
        if (!juniper_ok) {
            std::cout << "WARN - Juniper CQ failed\n";
            continue;
        }
        static_cast<JuniperStream *>(juniper_tag)->Srv::JuniperStream::Start();
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
        //std::cout << "Huawei: " << huawei_counter << std::endl;
        GPR_ASSERT(huawei_cq_->Next(&huawei_tag, &huawei_ok));
        //GPR_ASSERT(huawei_ok);
        if (!huawei_ok) {
            std::cout << "WARN - Huawei CQ failed\n";
            continue;
        }
        static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Start();
        //huawei_counter++;
    }
}

Srv::CiscoStream::CiscoStream(
                    mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
                    grpc::ServerCompletionQueue *cisco_cq) :
                                        cisco_service_ {cisco_service},
                                        cisco_cq_ {cisco_cq},
                                        cisco_resp {&cisco_server_ctx},
                                        cisco_stream_status {START}
{
    Srv::CiscoStream::Start();
}

Srv::JuniperStream::JuniperStream(
                    Subscriber::AsyncService *juniper_service,
                    grpc::ServerCompletionQueue *juniper_cq) :
                                        juniper_service_ {juniper_service},
                                        juniper_cq_ {juniper_cq},
                                        juniper_resp {&juniper_server_ctx},
                                        juniper_stream_status {START}
{
    Srv::JuniperStream::Start();
}

Srv::HuaweiStream::HuaweiStream(
                huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
                grpc::ServerCompletionQueue *huawei_cq) :
                                        huawei_service_ {huawei_service},
                                        huawei_cq_ {huawei_cq},
                                        huawei_resp {&huawei_server_ctx},
                                        huawei_stream_status {START}
{
    Srv::HuaweiStream::Start();
}

// --- Required for config parameters ---
std::unique_ptr<DataManipulationCfgHandler>
    data_manipulation_cfg_handler(new DataManipulationCfgHandler());
std::string enable_cisco_gpbkv2json =
    data_manipulation_cfg_handler->get_enable_cisco_gpbkv2json();
std::string enable_cisco_message_to_json_string =
    data_manipulation_cfg_handler->get_enable_cisco_message_to_json_string();
std::string enable_label_encode_as_map =
    data_manipulation_cfg_handler->get_enable_label_encode_as_map();
// --- Required for config parameters ---

void Srv::CiscoStream::Start()
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
        bool parsing_str {false};
        // From the network
        std::string stream_data_in;
        // After data enrichment
        std::string stream_data_out;
        std::string peer = cisco_server_ctx.peer();

        std::unique_ptr<DataManipulation> data_manipulation(
            new DataManipulation());
        std::unique_ptr<DataDelivery> data_delivery(
            new DataDelivery());
        std::unique_ptr<cisco_telemetry::Telemetry> cisco_tlm(
            new cisco_telemetry::Telemetry());

        new Srv::CiscoStream(cisco_service_, cisco_cq_);

        // the key-word "this" is used as a unique TAG
        cisco_resp.Read(&cisco_stream, this);
        // returns true for GPB-KV & GPB, false for JSON (from protobuf libs)
        parsing_str = cisco_tlm->ParseFromString(cisco_stream.data());

        stream_data_in = cisco_stream.data();

        // Handling empty data
        if (stream_data_in.empty()) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " CISCO Handling empty data: " << type_info
                                                                << std::endl;
            // ---

        // Handling GPB-KV
        } else if (cisco_tlm->data_gpbkv().empty() == false and
            parsing_str == true) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " CISCO Handling GPB-KV: " << type_info
                                                            << std::endl;
            // ---

            if (enable_cisco_gpbkv2json.compare("true") == 0) {
                data_manipulation->cisco_gpbkv2json(cisco_tlm, stream_data_in);
            } else if (enable_cisco_message_to_json_string.compare("true")
                                                                    == 0) {
                // MessageToJson is working directly on the PROTO-Obj
                stream_data_in.clear();
                google::protobuf::util::JsonPrintOptions opt;
                opt.add_whitespace = true;
                google::protobuf::util::MessageToJsonString(
                                                            *cisco_tlm,
                                                            &stream_data_in,
                                                            opt);
                // Data enrichment with label (node_id/platform_id)
                if (enable_label_encode_as_map.compare("true") == 0) {
                    if (data_manipulation->append_label_map(stream_data_in,
                            stream_data_out) == 0) {
                        data_delivery->async_kafka_producer(stream_data_out);
                    }
                } else {
                    stream_data_out = stream_data_in;
                    data_delivery->async_kafka_producer(stream_data_out);
                }
            } else {
                // Use Case: both data manipulation funcs set to false:
                // TBD: at the meoment simply send binary format to stdout
                std::cout << stream_data_in << std::endl;
            }

        // Handling GPB
        } else if (cisco_tlm->has_data_gpb() == true and parsing_str == true) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " CISCO Handling GPB: " << type_info
                                                        << std::endl;
            // ---

            // TBD

        // Handling JSON string
        } else if (parsing_str == false) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " CISCO Handling JSON string: " << type_info
                                                                << std::endl;
            // ---

            // Data enrichment with label (node_id/platform_id)
            if (enable_label_encode_as_map.compare("true") == 0) {
                if (data_manipulation->append_label_map(stream_data_in,
                        stream_data_out) == 0) {
                    data_delivery->async_kafka_producer(stream_data_out);
                }
            } else {
                stream_data_out = stream_data_in;
                data_delivery->async_kafka_producer(stream_data_out);
            }
        }
        // Memory leaks to be fixed
        //cisco_stream_status = END;
    } else if (cisco_stream_status == END) {
        cisco_resp.Finish(grpc::Status::OK, this);
        cisco_stream_status = DELETE;
    } else if (cisco_stream_status == DELETE) {
        delete this;
    }
}

void Srv::JuniperStream::Start()
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
        // From the network
        std::string stream_data_in;
        // After data enrichment
        std::string stream_data_out;
        std::string json_str_out;
        std::string peer = juniper_server_ctx.peer();
        Json::Value root;

        std::unique_ptr<DataManipulation> data_manipulation(
            new DataManipulation());
        std::unique_ptr<DataDelivery> data_delivery(
            new DataDelivery());
        std::unique_ptr<GnmiJuniperTelemetryHeaderExtension> juniper_tlm_hdr_ext(
            new GnmiJuniperTelemetryHeaderExtension());

        new Srv::JuniperStream(juniper_service_, juniper_cq_);

        // the key-word "this" is used as a unique TAG
        juniper_resp.Read(&juniper_stream, this);

        if (data_manipulation->juniper_extension(juniper_stream,
            juniper_tlm_hdr_ext, root) == 0 and
            data_manipulation->juniper_update(juniper_stream, json_str_out,
                root) == 0) {
                // to be properly logged
                std::cout << peer << " Juniper ext parsing succesful" << "\n";
        } else {
                // to be properly logged
                std::cout << peer << " Juniper ext parsing unsuccesful" << "\n";
        }

        // Data enrichment with label (node_id/platform_id)
        stream_data_in = json_str_out;
        if (enable_label_encode_as_map.compare("true") == 0) {
            if (data_manipulation->append_label_map(stream_data_in,
                    stream_data_out) == 0) {
                data_delivery->async_kafka_producer(stream_data_out);
            }
        } else {
            stream_data_out = json_str_out;
            data_delivery->async_kafka_producer(stream_data_out);
        }
        // Memory leaks to be fixed
        //juniper_stream_status = END;
    } else if (juniper_stream_status == END) {
        juniper_resp.Finish(grpc::Status::OK, this);
        juniper_stream_status = DELETE;
    } else if (juniper_stream_status == DELETE) {
        delete this;
    }
}

void Srv::HuaweiStream::Start()
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
        bool parsing_str {false};
        // From the network
        std::string stream_data_in;
        // Afetr data enrichment
        std::string stream_data_out;
        std::string json_str_out;
        std::string peer = huawei_server_ctx.peer();

        std::unique_ptr<DataManipulation> data_manipulation(
            new DataManipulation());
        std::unique_ptr<DataDelivery> data_delivery(
            new DataDelivery());
        std::unique_ptr<huawei_telemetry::Telemetry> huawei_tlm(
            new huawei_telemetry::Telemetry());
        std::unique_ptr<openconfig_interfaces::Interfaces> oc_if(
            new openconfig_interfaces::Interfaces());

        new Srv::HuaweiStream(huawei_service_, huawei_cq_);

        huawei_resp.Read(&huawei_stream, this);
        parsing_str = huawei_tlm->ParseFromString(huawei_stream.data());

        stream_data_in = huawei_stream.data();

        // Handling empty data
        if (stream_data_in.empty()) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " HUAWEI Handling empty data: " << type_info
                                                                << std::endl;
            // ---
        }

        // Handling GPB
        else {
            if (huawei_tlm->has_data_gpb() == true and parsing_str == true) {
                // ---
                auto type_info = typeid(stream_data_in).name();
                std::cout << peer << " HUAWEI Handling GPB: " << type_info
                                                                << std::endl;
                // ---
                stream_data_in.clear();
                google::protobuf::util::JsonPrintOptions opt;
                opt.add_whitespace = true;
                google::protobuf::util::MessageToJsonString(
                                                            *huawei_tlm,
                                                            &stream_data_in,
                                                            opt);
                // --- OC-IF ---
                int counter = 0;
                int rows = huawei_tlm->data_gpb().row_size();
                std::cout << "------- sensor_path: "
                    << huawei_tlm->sensor_path()
                    << " ------- rows: " << rows << "\n";

                bool parsing_content {false};
                std::string content_s;
                Json::Value root;

                root["sensor_path"] = huawei_tlm->sensor_path();

                while (counter < rows) {
                    content_s.clear();
                    std::cout << "------- row: " << counter << " -------\n";
                    std::string content = huawei_tlm->
                        data_gpb().row().at(counter).content();
                    parsing_content = oc_if->ParseFromString(content);
                    if (parsing_content == true) {
                        google::protobuf::util::JsonPrintOptions opt;
                        opt.add_whitespace = true;
                        google::protobuf::util::MessageToJsonString(
                                                            *oc_if,
                                                            &content_s,
                                                            opt);
                    }
                    root.append(content_s);

                    // Serialize the JSON value into a string
                    Json::StreamWriterBuilder builderW;
                    builderW["emitUTF8"] = false;
                    builderW["indentation"] = "";
                    const std::unique_ptr<Json::StreamWriter> writer(
                        builderW.newStreamWriter());
                    json_str_out = Json::writeString(builderW, root);

                    //std::cout << content_s << "\n";
                    counter++;
                }
                // --- OC-IF ---
            }

            // Data enrichment with label (node_id/platform_id)
            if (enable_label_encode_as_map.compare("true") == 0) {
                if (data_manipulation->append_label_map(stream_data_in,
                        stream_data_out) == 0) {
                    data_delivery->async_kafka_producer(stream_data_out);
                    data_delivery->async_kafka_producer(json_str_out);
                }
            } else {
                stream_data_out = stream_data_in;
                data_delivery->async_kafka_producer(stream_data_out);
                data_delivery->async_kafka_producer(json_str_out);
            }
        }

        stream_data_in = huawei_stream.data_json();

        // Handling empty data_json
        if (stream_data_in.empty()) {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " HUAWEI Handling empty data_json: "
                                                                << type_info
                                                                << std::endl;
            // ---
        }
        // Handling JSON string
        else {
            // ---
            auto type_info = typeid(stream_data_in).name();
            std::cout << peer << " HUAWEI Handling JSON string: " << type_info
                                                                << std::endl;
            // ---

            // Data enrichment with label (node_id/platform_id)
            if (enable_label_encode_as_map.compare("true") == 0) {
                if (data_manipulation->append_label_map(stream_data_in,
                        stream_data_out) == 0) {
                    data_delivery->async_kafka_producer(stream_data_out);
                }
            } else {
                stream_data_out = stream_data_in;
                data_delivery->async_kafka_producer(stream_data_out);
            }
        }
        // Memory leaks to be fixed
        //huawei_stream_status = END;
    } else if (huawei_stream_status == END) {
        huawei_resp.Finish(grpc::Status::OK, this);
        huawei_stream_status = DELETE;
    } else if (huawei_stream_status == DELETE) {
        delete this;
    }
}

