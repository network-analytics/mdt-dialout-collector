// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _SRV_H_
#define _SRV_H_

// C++ Standard Library headers
#include <typeinfo>
// External Library headers & System headers
#include <sys/socket.h>
#include <grpcpp/grpcpp.h>
#include <grpc/support/alloc.h>

#include "grpc/socket_mutator.h"
// mdt-dialout-collector Library headers
#include "proto/Cisco/cisco_dialout.grpc.pb.h"
#include "proto/Huawei/huawei_dialout.grpc.pb.h"
#include "proto/Juniper/juniper_dialout.grpc.pb.h"
#include "proto/Juniper/juniper_gnmi.pb.h"
#include "../dataManipulation/data_manipulation.h"
#include "../dataWrapper/data_wrapper.h"
#include "../dataDelivery/kafka_delivery.h"
#include "../dataDelivery/zmq_delivery.h"
#include "../utils/logs_handler.h"


// Global visibility to be able to signal the refresh --> CSV/PTM from main
extern std::unordered_map<std::string,std::vector<std::string>> label_map;

class ServerBuilderOptionImpl: public grpc::ServerBuilderOption {
public:
    ServerBuilderOptionImpl() {
        spdlog::get("multi-logger")->
            debug("constructor: ServerBuilderOptionImpl()"); };
    ~ServerBuilderOptionImpl() {
        spdlog::get("multi-logger")->
            debug("destructor: ~ServerBuilderOptionImpl()"); };
    virtual void UpdateArguments(grpc::ChannelArguments *args);
    virtual void UpdatePlugins(
        std::vector<std::unique_ptr<grpc::ServerBuilderPlugin>> *plugins) {}
};

class CustomSocketMutator: public grpc_socket_mutator {
public:
    CustomSocketMutator();
    ~CustomSocketMutator() {
        spdlog::get("multi-logger")->
            debug("destructor: ~CustomSocketMutator()"); };
    bool bindtodevice_socket_mutator(int fd);
};

class Srv final {
public:
    ~Srv()
    {
        spdlog::get("multi-logger")->debug("destructor: ~Srv()");
        cisco_server_->grpc::ServerInterface::Shutdown();
        juniper_server_->grpc::ServerInterface::Shutdown();
        huawei_server_->grpc::ServerInterface::Shutdown();
        cisco_cq_->grpc::ServerCompletionQueue::Shutdown();
        juniper_cq_->grpc::ServerCompletionQueue::Shutdown();
        huawei_cq_->grpc::ServerCompletionQueue::Shutdown();
    }
    Srv() { spdlog::get("multi-logger")->debug("constructor: Srv()"); };
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

    class CiscoStream {
    public:
        ~CiscoStream() { spdlog::get("multi-logger")->
            debug("destructor: ~CiscoStream()"); };
        CiscoStream(
            mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
            grpc::ServerCompletionQueue *cisco_cq);
        void Start(
            std::unordered_map<std::string,std::vector<std::string>>
                &label_map,
            DataManipulation &data_manipulation,
            DataWrapper &data_wrapper,
            KafkaDelivery &kafka_delivery,
            kafka::clients::KafkaProducer &producer,
            ZmqPush &zmq_pusher,
            cisco_telemetry::Telemetry &cisco_tlm
        );
    private:
        mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service_;
        grpc::ServerCompletionQueue *cisco_cq_;
        grpc::ServerContext cisco_server_ctx;
        mdt_dialout::MdtDialoutArgs cisco_stream;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
            mdt_dialout::MdtDialoutArgs> cisco_resp;
        int cisco_replies_sent;
        const int kCiscoMaxReplies;
        enum StreamStatus { START, FLOW, PROCESSING, END };
        StreamStatus cisco_stream_status;
    };

    class JuniperStream {
    public:
        ~JuniperStream() {
            spdlog::get("multi-logger")->
                debug("destructor: ~JuniperStream()"); };
        JuniperStream(
            Subscriber::AsyncService *juniper_service,
            grpc::ServerCompletionQueue *juniper_cq);
        void Start(
            std::unordered_map<std::string,std::vector<std::string>>
                &label_map,
            DataManipulation &data_manipulation,
            DataWrapper &data_wrapper,
            KafkaDelivery &kafka_delivery,
            kafka::clients::KafkaProducer &producer,
            ZmqPush &zmq_pusher,
            GnmiJuniperTelemetryHeaderExtension &juniper_tlm_hdr_ext
        );
    private:
        Subscriber::AsyncService *juniper_service_;
        grpc::ServerCompletionQueue *juniper_cq_;
        grpc::ServerContext juniper_server_ctx;
        gnmi::SubscribeResponse juniper_stream;
        grpc::ServerAsyncReaderWriter<gnmi::SubscribeRequest,
            gnmi::SubscribeResponse> juniper_resp;
        int juniper_replies_sent;
        const int kJuniperMaxReplies;
        enum StreamStatus { START, FLOW, PROCESSING, END };
        StreamStatus juniper_stream_status;
    };

    class HuaweiStream {
    public:
        ~HuaweiStream() {
            spdlog::get("multi-logger")->
                debug("destructor: ~HuaweiStream()"); };
        HuaweiStream(
            huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
            grpc::ServerCompletionQueue *huawei_cq);
        void Start(
            std::unordered_map<std::string,std::vector<std::string>>
                &label_map,
            DataManipulation &data_manipulation,
            DataWrapper &data_wrapper,
            KafkaDelivery &kafka_delivery,
            kafka::clients::KafkaProducer &producer,
            ZmqPush &zmq_pusher,
            huawei_telemetry::Telemetry &huawei_tlm,
            openconfig_interfaces::Interfaces &oc_if
        );
    private:
        huawei_dialout::gRPCDataservice::AsyncService *huawei_service_;
        grpc::ServerCompletionQueue *huawei_cq_;
        grpc::ServerContext huawei_server_ctx;
        huawei_dialout::serviceArgs huawei_stream;
        grpc::ServerAsyncReaderWriter<huawei_dialout::serviceArgs,
            huawei_dialout::serviceArgs> huawei_resp;
        int huawei_replies_sent;
        const int kHuaweiMaxReplies;
        enum StreamStatus { START, FLOW, PROCESSING, END };
        StreamStatus huawei_stream_status;
    };
};

#endif

