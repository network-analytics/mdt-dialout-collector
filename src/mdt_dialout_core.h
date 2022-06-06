#ifndef _SRV_H_
#define _SRV_H_

#include <iostream>
#include <grpcpp/grpcpp.h>
//#include <json/json.h>
#include "cisco_dialout.grpc.pb.h"
#include "huawei_dialout.grpc.pb.h"
#include "grpc/socket_mutator.h"


class ServerBuilderOptionImpl: public grpc::ServerBuilderOption {
public:
    virtual void UpdateArguments(grpc::ChannelArguments *args);
    virtual void UpdatePlugins(
        std::vector<std::unique_ptr<grpc::ServerBuilderPlugin>> *plugins) {}
private:
};

class CustomSocketMutator: public grpc_socket_mutator {
public:
    CustomSocketMutator();
    ~CustomSocketMutator() {}
    bool bindtodevice_socket_mutator(int fd);
private:
};

class SrvUtils {
public:
    int str2json(const std::string json_str);
    int async_kafka_prod(const std::string& json_str);
private:
};

class Srv final {
public:
    ~Srv();
    void CiscoBind(std::string cisco_srv_socket);
    void HuaweiBind(std::string huawei_srv_socket);

private:
    mdt_dialout::gRPCMdtDialout::AsyncService cisco_service_;
    huawei_dialout::gRPCDataservice::AsyncService huawei_service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cisco_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> huawei_cq_;
    std::unique_ptr<grpc::Server> cisco_server_;
    std::unique_ptr<grpc::Server> huawei_server_;
    void CiscoFsmCtrl();
    void HuaweiFsmCtrl();
    enum StreamStatus { START, FLOW, END };

    class CiscoStream: public SrvUtils {
    public:
        CiscoStream(
            mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
            grpc::ServerCompletionQueue *cisco_cq);
        void Start();
        void Stop();

    private:
        StreamStatus cisco_stream_status;
        mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service_;
        grpc::ServerCompletionQueue *cisco_cq_;
        grpc::ServerContext cisco_server_ctx;
        mdt_dialout::MdtDialoutArgs cisco_stream;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
                                    mdt_dialout::MdtDialoutArgs> cisco_resp;
    };

    class HuaweiStream: public SrvUtils {
    public:
        HuaweiStream(
            huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
            grpc::ServerCompletionQueue *huawei_cq);
        void Start();
        void Stop();

    private:
        StreamStatus huawei_stream_status;
        huawei_dialout::gRPCDataservice::AsyncService *huawei_service_;
        grpc::ServerCompletionQueue *huawei_cq_;
        grpc::ServerContext huawei_server_ctx;
        huawei_dialout::serviceArgs huawei_stream;
        grpc::ServerAsyncReaderWriter<huawei_dialout::serviceArgs,
                                    huawei_dialout::serviceArgs> huawei_resp;
    };
};

#endif

