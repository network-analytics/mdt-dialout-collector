#ifndef _SRV_H_
#define _SRV_H_

#include <iostream>
#include <unordered_map>
#include <vector>
#include <grpcpp/grpcpp.h>
#include <json/json.h>
#include "cisco_dialout.grpc.pb.h"
#include "cisco_telemetry.pb.h"
#include "huawei_dialout.grpc.pb.h"
#include "juniper_gnmi.pb.h"
#include "juniper_telemetry_header_extension.pb.h"
#include "juniper_dialout.grpc.pb.h"
#include "grpc/socket_mutator.h"


// Global visibility to be able to signal the refresh --> CSV from main
extern std::unordered_map<std::string,std::vector<std::string>> enrich_map;

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

class Srv final {
public:
    ~Srv()
    {
        std::cout << "~Srv()\n";
        cisco_server_->grpc::ServerInterface::Shutdown();
        juniper_server_->grpc::ServerInterface::Shutdown();
        huawei_server_->grpc::ServerInterface::Shutdown();
        cisco_cq_->grpc::ServerCompletionQueue::Shutdown();
        juniper_cq_->grpc::ServerCompletionQueue::Shutdown();
        huawei_cq_->grpc::ServerCompletionQueue::Shutdown();
    }
    //~Srv();
    void CiscoBind(std::string cisco_srv_socket);
    void JuniperBind(std::string juniper_srv_socket);
    void HuaweiBind(std::string huawei_srv_socket);

private:
    mdt_dialout::gRPCMdtDialout::AsyncService cisco_service_;
    Subscriber::AsyncService juniper_service_;
    huawei_dialout::gRPCDataservice::AsyncService huawei_service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cisco_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> juniper_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> huawei_cq_;
    std::unique_ptr<grpc::Server> cisco_server_;
    std::unique_ptr<grpc::Server> juniper_server_;
    std::unique_ptr<grpc::Server> huawei_server_;
    void CiscoFsmCtrl();
    void JuniperFsmCtrl();
    void HuaweiFsmCtrl();
    enum StreamStatus { START, FLOW, END };

    class CiscoStream {
    public:
        ~CiscoStream() { std::cout << "~CiscoStream()\n"; };
        CiscoStream(
            mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
            grpc::ServerCompletionQueue *cisco_cq);
        void Start(
        std::unordered_map<std::string,std::vector<std::string>>& enrich_map
        );

    private:
        mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service_;
        grpc::ServerCompletionQueue *cisco_cq_;
        grpc::ServerContext cisco_server_ctx;
        mdt_dialout::MdtDialoutArgs cisco_stream;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
            mdt_dialout::MdtDialoutArgs> cisco_resp;
        int cisco_init_counts;
        StreamStatus cisco_stream_status;
    };

    class JuniperStream {
    public:
        ~JuniperStream() {std::cout << "~JuniperStream()\n"; };
        JuniperStream(
            Subscriber::AsyncService *juniper_service,
            grpc::ServerCompletionQueue *juniper_cq);
        void Start(
        std::unordered_map<std::string,std::vector<std::string>>& enrich_map
        );

    private:
        Subscriber::AsyncService *juniper_service_;
        grpc::ServerCompletionQueue *juniper_cq_;
        grpc::ServerContext juniper_server_ctx;
        gnmi::SubscribeResponse juniper_stream;
        grpc::ServerAsyncReaderWriter<gnmi::SubscribeRequest,
            gnmi::SubscribeResponse> juniper_resp;
        int juniper_init_counts;
        StreamStatus juniper_stream_status;
    };

    class HuaweiStream {
    public:
        ~HuaweiStream() {std::cout << "~HuaweiStream()\n"; };
        HuaweiStream(
            huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
            grpc::ServerCompletionQueue *huawei_cq);
        void Start(
        std::unordered_map<std::string,std::vector<std::string>>& enrich_map
        );

    private:
        huawei_dialout::gRPCDataservice::AsyncService *huawei_service_;
        grpc::ServerCompletionQueue *huawei_cq_;
        grpc::ServerContext huawei_server_ctx;
        huawei_dialout::serviceArgs huawei_stream;
        grpc::ServerAsyncReaderWriter<huawei_dialout::serviceArgs,
            huawei_dialout::serviceArgs> huawei_resp;
        int huawei_init_counts;
        StreamStatus huawei_stream_status;
    };
};

#endif

