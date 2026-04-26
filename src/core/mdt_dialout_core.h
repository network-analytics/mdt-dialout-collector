// Copyright(c) 2022-2025, Salvatore Cuzzilla (Swisscom AG)
// Copyright(c) 2026-present, Salvatore Cuzzilla (Avaloq, an NEC Company)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


#ifndef _SRV_H_
#define _SRV_H_

#include <atomic>
#include <shared_mutex>
#include <typeinfo>
#include <vector>
#include <sys/socket.h>
#include <grpcpp/grpcpp.h>
#include <grpc/support/alloc.h>

#include "grpc/socket_mutator.h"
#include "proto/Cisco/cisco_dialout.grpc.pb.h"
#include "proto/Huawei/huawei_dialout.grpc.pb.h"
#include "proto/Juniper/juniper_dialout.grpc.pb.h"
#include "proto/Juniper/juniper_gnmi.pb.h"
#include "proto/Nokia/nokia_dialout.grpc.pb.h"
#include "proto/Nokia/nokia_gnmi.pb.h"
#include "../dataManipulation/data_manipulation.h"
#include "../dataWrapper/data_wrapper.h"
#include "../dataDelivery/kafka_delivery.h"
#include "../dataDelivery/zmq_delivery.h"
#include "../utils/logs_handler.h"


// label_map: reader = vendor HandleMessage (shared_lock); writer = signal
// watcher on SIGUSR1 (unique_lock). The handler only flips the flag —
// file I/O and allocation are not async-signal-safe.
extern std::unordered_map<std::string,std::vector<std::string>> label_map;
extern std::shared_mutex label_map_mutex;
extern std::atomic<bool> label_map_reload_pending;

enum class DeliveryMethod { Kafka, Zmq };

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
    void log_socket_options(int fd);
};

class Srv final {
public:
    enum class Vendor { Unset, Cisco, Juniper, Nokia, Huawei };

    ~Srv()
    {
        spdlog::get("multi-logger")->debug("destructor: ~Srv()");
        Shutdown();
    }
    Srv() { spdlog::get("multi-logger")->debug("constructor: Srv()"); };
    void CiscoBind(std::string cisco_srv_socket);
    void JuniperBind(std::string juniper_srv_socket);
    void NokiaBind(std::string nokia_srv_socket);
    void HuaweiBind(std::string huawei_srv_socket);

    // Idempotent; drains up to 5s, then shuts the active CQ.
    void Shutdown();

private:
    Vendor            active_vendor_ {Vendor::Unset};
    std::atomic<bool> shutdown_done_ {false};

    mdt_dialout::gRPCMdtDialout::AsyncService cisco_service_;
    Subscriber::AsyncService juniper_service_;
    Nokia::SROS::DialoutTelemetry::AsyncService nokia_service_;
    huawei_dialout::gRPCDataservice::AsyncService huawei_service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cisco_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> juniper_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> nokia_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> huawei_cq_;
    std::unique_ptr<grpc::Server> cisco_server_;
    std::unique_ptr<grpc::Server> juniper_server_;
    std::unique_ptr<grpc::Server> nokia_server_;
    std::unique_ptr<grpc::Server> huawei_server_;
    void CiscoFsmCtrl();
    void JuniperFsmCtrl();
    void NokiaFsmCtrl();
    void HuaweiFsmCtrl();

    class CiscoStream {
    public:
        // Shared by every CiscoStream of one FsmCtrl; lives on its stack.
        struct Context {
            std::unordered_map<std::string, std::vector<std::string>>
                                            *label_map;
            DataManipulation                *data_manipulation;
            DataWrapper                     *data_wrapper;
            KafkaDelivery                   *kafka_delivery;
            kafka::clients::KafkaProducer   *kafka_producer;  // null in zmq mode
            ZmqPush                         *zmq_pusher;
            zmq::socket_t                   *zmq_sock;        // null in kafka mode
            std::string                      zmq_uri;
            int                              max_replies;     // 0 = unlimited
            DeliveryMethod                   delivery;
            std::string                      writer_id;
            bool                             label_encode;
            bool                             cisco_gpbkv2json;
            bool                             cisco_msg_to_json;
        };

        static void Spawn(
            mdt_dialout::gRPCMdtDialout::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void Proceed(bool ok);

        ~CiscoStream() { spdlog::get("multi-logger")->
            debug("destructor: ~CiscoStream()"); };
    private:
        enum class State { Create, Read, Finishing, Done };

        CiscoStream(
            mdt_dialout::gRPCMdtDialout::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void HandleMessage();

        mdt_dialout::gRPCMdtDialout::AsyncService *svc_;
        grpc::ServerCompletionQueue               *cq_;
        Context                                   *ctx_;
        grpc::ServerContext                        server_ctx_;
        mdt_dialout::MdtDialoutArgs                request_;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
            mdt_dialout::MdtDialoutArgs>           resp_;
        cisco_telemetry::Telemetry                 tlm_;
        State                                      state_;
        int                                        replies_;
    };

    class JuniperStream {
    public:
        struct Context {
            std::unordered_map<std::string, std::vector<std::string>>
                                            *label_map;
            DataManipulation                *data_manipulation;
            DataWrapper                     *data_wrapper;
            KafkaDelivery                   *kafka_delivery;
            kafka::clients::KafkaProducer   *kafka_producer;
            ZmqPush                         *zmq_pusher;
            zmq::socket_t                   *zmq_sock;
            std::string                      zmq_uri;
            int                              max_replies;
            DeliveryMethod                   delivery;
            std::string                      writer_id;
            bool                             label_encode;
        };

        static void Spawn(
            Subscriber::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void Proceed(bool ok);

        ~JuniperStream() {
            spdlog::get("multi-logger")->
                debug("destructor: ~JuniperStream()"); };
    private:
        enum class State { Create, Read, Finishing, Done };

        JuniperStream(
            Subscriber::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void HandleMessage();

        Subscriber::AsyncService                     *svc_;
        grpc::ServerCompletionQueue                  *cq_;
        Context                                      *ctx_;
        grpc::ServerContext                           server_ctx_;
        juniper_gnmi::SubscribeResponse               request_;
        grpc::ServerAsyncReaderWriter<
            juniper_gnmi::SubscribeRequest,
            juniper_gnmi::SubscribeResponse>          resp_;
        GnmiJuniperTelemetryHeaderExtension           tlm_hdr_ext_;
        State                                         state_;
        int                                           replies_;
    };

    class NokiaStream {
    public:
        struct Context {
            std::unordered_map<std::string, std::vector<std::string>>
                                            *label_map;
            DataManipulation                *data_manipulation;
            DataWrapper                     *data_wrapper;
            KafkaDelivery                   *kafka_delivery;
            kafka::clients::KafkaProducer   *kafka_producer;
            ZmqPush                         *zmq_pusher;
            zmq::socket_t                   *zmq_sock;
            std::string                      zmq_uri;
            int                              max_replies;
            DeliveryMethod                   delivery;
            std::string                      writer_id;
            bool                             label_encode;
        };

        static void Spawn(
            Nokia::SROS::DialoutTelemetry::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void Proceed(bool ok);

        ~NokiaStream() {
            spdlog::get("multi-logger")->
                debug("destructor: ~NokiaStream()"); };
    private:
        enum class State { Create, Read, Finishing, Done };

        NokiaStream(
            Nokia::SROS::DialoutTelemetry::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void HandleMessage();

        Nokia::SROS::DialoutTelemetry::AsyncService  *svc_;
        grpc::ServerCompletionQueue                  *cq_;
        Context                                      *ctx_;
        grpc::ServerContext                           server_ctx_;
        nokia_gnmi::SubscribeResponse                 request_;
        grpc::ServerAsyncReaderWriter<
            Nokia::SROS::PublishResponse,
            nokia_gnmi::SubscribeResponse>            resp_;
        State                                         state_;
        int                                           replies_;
    };

    class HuaweiStream {
    public:
        struct Context {
            std::unordered_map<std::string, std::vector<std::string>>
                                            *label_map;
            DataManipulation                *data_manipulation;
            DataWrapper                     *data_wrapper;
            KafkaDelivery                   *kafka_delivery;
            kafka::clients::KafkaProducer   *kafka_producer;
            ZmqPush                         *zmq_pusher;
            zmq::socket_t                   *zmq_sock;
            std::string                      zmq_uri;
            int                              max_replies;
            DeliveryMethod                   delivery;
            std::string                      writer_id;
            bool                             label_encode;
        };

        static void Spawn(
            huawei_dialout::gRPCDataservice::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void Proceed(bool ok);

        ~HuaweiStream() {
            spdlog::get("multi-logger")->
                debug("destructor: ~HuaweiStream()"); };
    private:
        enum class State { Create, Read, Finishing, Done };

        HuaweiStream(
            huawei_dialout::gRPCDataservice::AsyncService *svc,
            grpc::ServerCompletionQueue *cq,
            Context *ctx);

        void HandleMessage();

        huawei_dialout::gRPCDataservice::AsyncService *svc_;
        grpc::ServerCompletionQueue                   *cq_;
        Context                                       *ctx_;
        grpc::ServerContext                            server_ctx_;
        huawei_dialout::serviceArgs                    request_;
        grpc::ServerAsyncReaderWriter<
            huawei_dialout::serviceArgs,
            huawei_dialout::serviceArgs>               resp_;
        huawei_telemetry::Telemetry                    tlm_;
        openconfig_interfaces::Interfaces              oc_if_;
        State                                          state_;
        int                                            replies_;
    };
};

// Shutdown registry: each Srv self-registers; signal-watcher calls
// initiate_shutdown() to drain all live servers on SIGTERM/SIGINT.
void register_server(Srv *srv);
void unregister_server(Srv *srv);
void initiate_shutdown();

#endif

