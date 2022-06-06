#include <iostream>
#include <fstream>
#include <thread>
#include <typeinfo>
#include <grpcpp/grpcpp.h>
#include "grpc/socket_mutator.h"
#include <grpcpp/support/channel_arguments.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpcpp/impl/server_builder_plugin.h>
#include <json/json.h>
//#include <librdkafka/rdkafkacpp.h>
#include "kafka/KafkaProducer.h"
#include "mdt_dialout_core.h"
#include "cisco_dialout.grpc.pb.h"
#include "cisco_telemetry.pb.h"
#include "huawei_dialout.grpc.pb.h"
#include "huawei_telemetry.pb.h"
#include <google/protobuf/arena.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


bool CustomSocketMutator::bindtodevice_socket_mutator(int fd)
{
    int type;
    int length = sizeof(int);
    socklen_t len = sizeof(type);

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) != 0) {
        //std::cout << "Issues with getting the iface type ..." << std::endl;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                                "vrf300", strlen("vrf300")) != 0) {
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
    huawei_server_->grpc::ServerInterface::Shutdown();
    cisco_cq_->grpc::ServerCompletionQueue::Shutdown();
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

    std::thread t1(&Srv::CiscoFsmCtrl, this);
    std::thread t2(&Srv::CiscoFsmCtrl, this);
    std::thread t3(&Srv::CiscoFsmCtrl, this);

    t1.join();
    t2.join();
    t3.join();
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

    std::thread t1(&Srv::HuaweiFsmCtrl, this);
    std::thread t2(&Srv::HuaweiFsmCtrl, this);
    std::thread t3(&Srv::HuaweiFsmCtrl, this);

    t1.join();
    t2.join();
    t3.join();
}

void Srv::CiscoFsmCtrl()
{
    new Srv::CiscoStream(&cisco_service_, cisco_cq_.get());
    int cisco_counter {0};
    void *cisco_tag {nullptr};
    bool cisco_ok {false};
    while (true) {
        //std::cout << "Cisco: " << cisco_counter << std::endl;
        GPR_ASSERT(cisco_cq_->Next(&cisco_tag, &cisco_ok));
        if (!cisco_ok) {
            static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Stop();
            continue;
        }
        static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Start();
        //cisco_counter++;
    }
}

void Srv::HuaweiFsmCtrl()
{
    new Srv::HuaweiStream(&huawei_service_, huawei_cq_.get());
    int huawei_counter {0};
    void *huawei_tag {nullptr};
    bool huawei_ok {false};
    while (true) {
        //std::cout << "Huawei: " << huawei_counter << std::endl;
        GPR_ASSERT(huawei_cq_->Next(&huawei_tag, &huawei_ok));
        if (!huawei_ok) {
            static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Stop();
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
        //std::string peer = server_ctx.peer();
        //std::cout << "Peer: " + peer << std::endl;
        std::unique_ptr<Srv::CiscoStream> srv_utils(
                            new Srv::CiscoStream(cisco_service_, cisco_cq_));
        // the key-word "this" is used as a unique TAG
        cisco_resp.Read(&cisco_stream, this);

        /*
         * Srv::Stream::str2json(stream_data);
         * if (std::ofstream output{"gpbkv.bin", std::ios::app}) {
         *   output << stream.data();
         * }
         * else {
         *   std::exit(EXIT_FAILURE);
         * }
         */

        std::string stream_data;
        std::unique_ptr<google::protobuf::Message> cisco_tlm(
                                            new cisco_telemetry::Telemetry());

        // Handling empty data
        if (cisco_stream.data().empty()) {
            stream_data = "{ }";
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling empty data: " << type_info << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        // Handling GPB-KV
        } else if (cisco_tlm->ParseFromString(cisco_stream.data()) and stream_data != "{ }") {
            google::protobuf::util::JsonOptions opt;
            opt.add_whitespace = true;
            google::protobuf::util::MessageToJsonString(
                                                        *cisco_tlm,
                                                        &stream_data,
                                                        opt);
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling GPB-KV: " << type_info << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        // Handling JSON string
        } else {
            stream_data = cisco_stream.data();
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling JSON string: " << type_info << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        } 
    } else {
        GPR_ASSERT(cisco_stream_status == END);
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
        std::unique_ptr<Srv::HuaweiStream> srv_utils(
                        new Srv::HuaweiStream(huawei_service_, huawei_cq_));
        huawei_resp.Read(&huawei_stream, this);
        
        std::string stream_data;
        std::unique_ptr<google::protobuf::Message> huawei_tlm(
                                            new huawei_telemetry::Telemetry());

        // Handling empty data
        if (huawei_stream.data().empty()) {
            stream_data = "{ }";
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling empty data: " << type_info << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        }
        // Handling GPB-KV
        else {
            if (huawei_tlm->ParseFromString(huawei_stream.data())) {
                google::protobuf::util::JsonOptions opt;
                opt.add_whitespace = true;
                google::protobuf::util::MessageToJsonString(
                                                            *huawei_tlm,
                                                            &stream_data,
                                                            opt);
                // ---
                auto type_info = typeid(stream_data).name();
                std::cout << "Handling GPB-KV: " << type_info << std::endl;
                // ---
                srv_utils->str2json(stream_data);
                srv_utils->async_kafka_prod(stream_data);
            }
        }

        // Handling empty data_json
        if (huawei_stream.data_json().empty()) {
            stream_data = "{ }";
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling empty data_json: " << type_info 
                                                            << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        }    
        // Handling JSON string
        else {
            stream_data = huawei_stream.data_json();
            // ---
            auto type_info = typeid(stream_data).name();
            std::cout << "Handling JSON string: " << type_info << std::endl;
            // ---
            srv_utils->str2json(stream_data);
            srv_utils->async_kafka_prod(stream_data);
        }
    } else {
        GPR_ASSERT(huawei_stream_status == END);
        delete this;
    }
}

void Srv::CiscoStream::Stop()
{
    //std::cout << "Streaming Interrupted ..." << std::endl;
    cisco_stream_status = END;
}

void Srv::HuaweiStream::Stop()
{
    //std::cout << "Streaming Interrupted ..." << std::endl;
    huawei_stream_status = END;
}

/**
 * string-to-json can be used for data manipulation
 */
int SrvUtils::str2json(const std::string& json_str)
{
    const auto json_str_length = static_cast<int>(json_str.length());
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builderR;
    Json::StreamWriterBuilder builderW;
    const std::unique_ptr<Json::CharReader> reader(builderR.newCharReader());
    const std::unique_ptr<Json::StreamWriter> writer(
                                                builderW.newStreamWriter());
    if (!reader->parse(json_str.c_str(), json_str.c_str() + json_str_length,
                      &root, &err) and json_str_length != 0) {
        std::cout << "error" << std::endl;
        std::cout << "generating errors: " << json_str << std::endl;
        return EXIT_FAILURE;
    }

    //writer->write(root, &std::cout);
    //const std::string encoding_path = root["encoding_path"].asString();
    //const std::string msg_timestamp = root["msg_timestamp"].asString();
    //const std::string node_id_str = root["node_id_str"].asString();

    //std::cout << encoding_path << std::endl;
    //std::cout << msg_timestamp << std::endl;
    //std::cout << node_id_str << std::endl;

    return EXIT_SUCCESS;
}

int SrvUtils::async_kafka_prod(const std::string& json_str)
{
    using namespace kafka::clients;

    std::string brokers = "kafka.sbd.corproot.net:9093";
    kafka::Topic topic  = "daisy.dev.yang-json-raw-test";
    std::string client_id = "mdt-dialout-collector";

    try {
        // Additional config options here
        kafka::Properties properties ({
            {"bootstrap.servers",  brokers},
            {"enable.idempotence", "true"},
            {"client.id", client_id},
            {"security.protocol", "ssl"},
            {"ssl.key.location", "/opt/daisy/cert/dev/collectors.key"},
            {"ssl.certificate.location", "/opt/daisy/cert/dev/collectors.crt"},
            {"ssl.ca.location", "/opt/daisy/cert/dev/sbd_root_ca.crt"},
        });

        KafkaProducer producer(properties);

        if (json_str.empty()) {
            // Better handling
            std::cout << "Empty json rcv ..." << std::endl;
            return EXIT_FAILURE;
        }

        auto msg = producer::ProducerRecord(topic,
                    kafka::NullKey,
                    kafka::Value(json_str.c_str(), json_str.size()));

        producer.send(
            msg,
            [](const producer::RecordMetadata& mdata,
                const kafka::Error& err) {
            if (!err) {
                std::cout << "Msg delivered: "
                        << mdata.toString() << std::endl;
            } else {
                std::cerr << "Msg delivery failed: "
                        << err.message() << std::endl;
            }
        }, KafkaProducer::SendOption::ToCopyRecordValue);
    } catch (const kafka::KafkaException& ex) {
        std::cerr << "Unexpected exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

