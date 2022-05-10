#include <iostream>
#include <fstream>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <json/json.h>
//#include <librdkafka/rdkafkacpp.h>
#include "kafka/KafkaProducer.h"
#include "mdt_dialout_core.h"
#include "mdt_dialout.grpc.pb.h"
#include "telemetry.pb.h"
#include <google/protobuf/arena.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>


Srv::~Srv()
{
    server_->grpc::ServerInterface::Shutdown();
    cq_->grpc::ServerCompletionQueue::Shutdown();
}

void Srv::Bind(std::string srv_addr)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(srv_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();

    Srv::FsmCtrl();
    //std::thread t1(&Srv::FsmCtrl, this);
    //std::thread t2(&Srv::FsmCtrl, this);
    //std::thread t3(&Srv::FsmCtrl, this);

    //t1.join();
    //t2.join();
    //t3.join();
}

/**
 * Parallelism should be eventually handled with this func
 */
void Srv::FsmCtrl()
{
    new Srv::Stream(&service_, cq_.get());
    int counter {0};
    void *tag {nullptr};
    bool ok {false};
    while (true) {
        //std::cout << counter << std::endl;
        GPR_ASSERT(cq_->Next(&tag, &ok));
        //GPR_ASSERT(ok);
        if (!ok) {
            /* Something went wrong with CQ -> set stream_status = END */
            static_cast<Stream *>(tag)->Stop();
            continue;
        }
        static_cast<Stream *>(tag)->Start();
        counter++;
    }
}

Srv::Stream::Stream(mdt_dialout::gRPCMdtDialout::AsyncService *service,
                    grpc::ServerCompletionQueue *cq) : service_ {service},
                                                        cq_ {cq},
                                                        resp {&server_ctx},
                                                        stream_status {START}
{
    Srv::Stream::Start();
}

void Srv::Stream::Start()
{
    /**
     * Initial stream_status set to START
     */
    if (stream_status == START) {
        service_->RequestMdtDialout(&server_ctx, &resp, cq_, cq_, this);
        stream_status = FLOW;
    } else if (stream_status == FLOW) {
        //std::cout << "Streaming Started ..." << std::endl;
        //std::string peer = server_ctx.peer();
        //std::cout << "Peer: " + peer << std::endl;
        new Srv::Stream(service_, cq_);
        /* this is used as a unique TAG */
        resp.Read(&stream, this);
        std::string stream_data;
        //Srv::Stream::str2json(stream_data);
        //if (std::ofstream output{"gpbkv.bin", std::ios::app}) {
        //    output << stream.data();
        //} else {
        //    std::exit(EXIT_FAILURE);
        //}
        google::protobuf::Message *tlm = new telemetry::Telemetry;
        tlm->ParseFromString(stream.data());
        google::protobuf::util::JsonOptions opt;
        opt.add_whitespace = true;
        google::protobuf::util::MessageToJsonString(*tlm, &stream_data, opt);
        Srv::Stream::async_kafka_prod(stream_data);
        //std::cout << stream_data;
    } else {
        GPR_ASSERT(stream_status == END);
        delete this;
    }
}

void Srv::Stream::Stop()
{
    //std::cout << "Streaming Interrupted ..." << std::endl;
    stream_status = END;
}

/**
 * string-to-json can be used for data manipulation
 */
int Srv::Stream::str2json(const std::string& json_str)
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

int Srv::Stream::async_kafka_prod(const std::string& json_str)
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
