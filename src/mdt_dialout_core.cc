#include <iostream>
#include <fstream>
#include <thread>
#include <typeinfo>
#include <grpcpp/grpcpp.h>
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
    cisco_builder.AddListeningPort(cisco_srv_socket, grpc::InsecureServerCredentials());
    cisco_builder.RegisterService(&cisco_service_);
    cisco_cq_ = cisco_builder.AddCompletionQueue();
    cisco_server_ = cisco_builder.BuildAndStart();

    Srv::CiscoFsmCtrl();
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
    huawei_builder.AddListeningPort(huawei_srv_socket, grpc::InsecureServerCredentials());
    huawei_builder.RegisterService(&huawei_service_);
    huawei_cq_ = huawei_builder.AddCompletionQueue();
    huawei_server_ = huawei_builder.BuildAndStart();

    Srv::HuaweiFsmCtrl();
    std::thread t1(&Srv::HuaweiFsmCtrl, this);
    std::thread t2(&Srv::HuaweiFsmCtrl, this);
    std::thread t3(&Srv::HuaweiFsmCtrl, this);

    t1.join();
    t2.join();
    t3.join();
}

/**
 * Parallelism should be eventually handled with this func
 */
void Srv::CiscoFsmCtrl()
{
    new Srv::CiscoStream(&cisco_service_, cisco_cq_.get());
    int counter {0};
    void *cisco_tag {nullptr};
    bool cisco_ok {false};
    while (true) {
        //std::cout << counter << std::endl;
        GPR_ASSERT(cisco_cq_->Next(&cisco_tag, &cisco_ok));
        //GPR_ASSERT(ok);
        if (!cisco_ok) {
            /* Something went wrong with CQ -> set stream_status = END */
            static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Stop();
            continue;
        }
        static_cast<CiscoStream *>(cisco_tag)->Srv::CiscoStream::Start();
        counter++;
    }
}

void Srv::HuaweiFsmCtrl()
{
    new Srv::HuaweiStream(&huawei_service_, huawei_cq_.get());
    int counter {0};
    void *huawei_tag {nullptr};
    bool huawei_ok {false};
    while (true) {
        //std::cout << counter << std::endl;
        GPR_ASSERT(huawei_cq_->Next(&huawei_tag, &huawei_ok));
        //GPR_ASSERT(ok);
        if (!huawei_ok) {
            /* Something went wrong with CQ -> set stream_status = END */
            static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Stop();
            continue;
        }
        static_cast<HuaweiStream *>(huawei_tag)->Srv::HuaweiStream::Start();
        counter++;
    }
}

Srv::CiscoStream::CiscoStream(mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
                    grpc::ServerCompletionQueue *cisco_cq) : cisco_service_ {cisco_service},
                                                        cisco_cq_ {cisco_cq},
                                                        cisco_resp {&cisco_server_ctx},
                                                        cisco_stream_status {START}
{
    Srv::CiscoStream::Start();
}

Srv::HuaweiStream::HuaweiStream(huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
                    grpc::ServerCompletionQueue *huawei_cq) : huawei_service_ {huawei_service},
                                                        huawei_cq_ {huawei_cq},
                                                        huawei_resp {&huawei_server_ctx},
                                                        huawei_stream_status {START}
{
    Srv::HuaweiStream::Start();
}

void Srv::CiscoStream::Start()
{
    /**
     * Initial stream_status set to START
     */
    if (cisco_stream_status == START) {
        cisco_service_->RequestMdtDialout(&cisco_server_ctx, &cisco_resp, cisco_cq_, cisco_cq_, this);
        cisco_stream_status = FLOW;
    } else if (cisco_stream_status == FLOW) {
        //std::string peer = server_ctx.peer();
        //std::cout << "Peer: " + peer << std::endl;
        new Srv::CiscoStream(cisco_service_, cisco_cq_);
        /* this is used as a unique TAG */
        cisco_resp.Read(&cisco_stream, this);

        //auto type_info = typeid(stream.data()).name();
        //std::cout << type_info << std::endl;

        std::string stream_data;
        //Srv::Stream::str2json(stream_data);
        //if (std::ofstream output{"gpbkv.bin", std::ios::app}) {
        //    output << stream.data();
        //} else {
        //    std::exit(EXIT_FAILURE);
        //}
        google::protobuf::Message *cisco_tlm = new cisco_telemetry::Telemetry;
        if (cisco_tlm->ParseFromString(cisco_stream.data())) {
            google::protobuf::util::JsonOptions opt;
            opt.add_whitespace = true;
            google::protobuf::util::MessageToJsonString(*cisco_tlm, &stream_data, opt);
            //Srv::Stream::async_kafka_prod(stream_data);
            std::cout << stream_data << std::endl;
        } else {
            //Srv::Stream::async_kafka_prod(stream.data());
            std::cout << cisco_stream.data() << std::endl;
        }
    } else {
        GPR_ASSERT(cisco_stream_status == END);
        delete this;
    }
}

void Srv::HuaweiStream::Start()
{
    /**
     * Initial stream_status set to START
     */
    if (huawei_stream_status == START) {
        huawei_service_->RequestdataPublish(&huawei_server_ctx, &huawei_resp, huawei_cq_, huawei_cq_, this);
        huawei_stream_status = FLOW;
    } else if (huawei_stream_status == FLOW) {
        //std::cout << "Streaming Started ..." << std::endl;
        //std::string peer = server_ctx.peer();
        //std::cout << "Peer: " + peer << std::endl;
        new Srv::HuaweiStream(huawei_service_, huawei_cq_);
        /* this is used as a unique TAG */
        huawei_resp.Read(&huawei_stream, this);

        /**
         * Huawei JSON format
         */
        std::cout << huawei_stream.data_json() << std::endl;

        //auto type_info = typeid(stream.data()).name();
        //std::cout << type_info << std::endl;

        std::string stream_data;
        //Srv::Stream::str2json(stream_data);
        //if (std::ofstream output{"gpbkv.bin", std::ios::app}) {
        //    output << stream.data();
        //} else {
        //    std::exit(EXIT_FAILURE);
        //}
        google::protobuf::Message *huawei_tlm = new huawei_telemetry::Telemetry;
        if (huawei_tlm->ParseFromString(huawei_stream.data())) {
            google::protobuf::util::JsonOptions opt;
            opt.add_whitespace = true;
            google::protobuf::util::MessageToJsonString(*huawei_tlm, &stream_data, opt);
            //Srv::Stream::async_kafka_prod(stream_data);
            std::cout << stream_data << std::endl;
        } else {
            //Srv::Stream::async_kafka_prod(stream.data());
            std::cout << huawei_stream.data_json() << std::endl;
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
int Srv::CiscoStream::str2json(const std::string& json_str)
{
    const auto json_str_length = static_cast<int>(json_str.length());
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builderR;
    Json::StreamWriterBuilder builderW;
    const std::unique_ptr<Json::CharReader> reader(builderR.newCharReader());
    const std::unique_ptr<Json::StreamWriter> writer(builderW.newStreamWriter());
    if (!reader->parse(json_str.c_str(), json_str.c_str() + json_str_length,
                      &root, &err)) {
        std::cout << "error" << std::endl;
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

int Srv::CiscoStream::async_kafka_prod(const std::string& json_str)
{
    using namespace kafka::clients;

    std::string brokers = "192.168.100.241";
    kafka::Topic topic  = "quickstart";

    try {
        // Additional config options here
        kafka::Properties properties ({
            {"bootstrap.servers",  brokers},
            {"enable.idempotence", "true"},
        });

        KafkaProducer producer(properties);

        if (json_str.empty()) {
            // TBD
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
