#include <iostream>
#include <grpcpp/grpcpp.h>
#include <json/json.h>
#include "mdt_dialout_core.h"
#include "mdt_dialout.grpc.pb.h"


Srv::~Srv()
{
    server_->grpc::ServerInterface::Shutdown();
    cq_->grpc::CompletionQueue::Shutdown();
}

void Srv::Bind(std::string srv_addr)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(srv_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    Srv::FsmCtrl();
}

/* Parallelism should be eventually handled with this func */
void Srv::FsmCtrl()
{
    new Srv::Stream(&service_, cq_.get());
    void *tag {nullptr};
    bool ok {false};
    while (true) {
        GPR_ASSERT(cq_->Next(&tag, &ok));
        if (!ok) {
            /* Something went wrong with CQ -> set stream_status = END */
            static_cast<Stream *>(tag)->Stop();
            continue;
        }
        static_cast<Stream *>(tag)->Start();
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
    const std::string encoding_path = root["encoding_path"].asString();
    const std::string msg_timestamp = root["msg_timestamp"].asString();
    const std::string node_id_str = root["node_id_str"].asString();

    std::cout << encoding_path << std::endl;
    std::cout << msg_timestamp << std::endl;
    std::cout << node_id_str << std::endl;

    return EXIT_SUCCESS;
}

void Srv::Stream::Start()
{
    /* Initial stream_status set to START */
    if (stream_status == START) {
        service_->RequestMdtDialout(&server_ctx, &resp, cq_, cq_, this);
        stream_status = FLOW;
    } else if (stream_status == FLOW) {
        //std::cout << "Streaming Started ..." << std::endl;
        //std::string peer = server_ctx.peer();
        //std::cout << "Peer: " + peer << std::endl;
        new Srv::Stream(service_, cq_);
        resp.Read(&stream, this);
        const std::string stream_data = stream.data();
        Srv::Stream::str2json(stream_data);
        //std::cout << stream.data() << std::endl;
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

