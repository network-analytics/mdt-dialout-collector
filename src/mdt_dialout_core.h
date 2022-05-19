#ifndef _SRV_H_
#define _SRV_H_

#include <iostream>
#include <grpcpp/grpcpp.h>
//#include <json/json.h>
#include "cisco_dialout.grpc.pb.h"
#include "huawei_dialout.grpc.pb.h"


/**
 * Prefix each Class-name or Method-name with MdtDialout
 * should make the code more readable
 */
class Srv final {
public:
    ~Srv();
    void Bind(std::string srv_addr);

private:
    cisco_dialout::gRPCMdtDialout::AsyncService cisco_service_;
    huawei_dialout::gRPCDataservice::AsyncService huawei_service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::unique_ptr<grpc::Server> server_;
    void FsmCtrl();

    class CiscoStream {
    public:
        CiscoStream(cisco_dialout::gRPCMdtDialout::AsyncService *cisco_service,
            grpc::ServerCompletionQueue *cq);
        enum StreamStatus { START, FLOW, END };
        StreamStatus stream_status;
        void Start();
        void Stop();
        int str2json(const std::string& json_str);
        int async_kafka_prod(const std::string& json_str);

    private:
        cisco_dialout::gRPCMdtDialout::AsyncService *cisco_service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext server_ctx;
        cisco_dialout::MdtDialoutArgs cisco_stream;
        grpc::ServerAsyncReaderWriter<cisco_dialout::MdtDialoutArgs,
                                    cisco_dialout::MdtDialoutArgs> cisco_resp;
    };

    class HuaweiStream {
    public:
        HuaweiStream(huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
            grpc::ServerCompletionQueue *cq);
        enum StreamStatus { START, FLOW, END };
        StreamStatus stream_status;
        void Start();
        void Stop();
        //int str2json(const std::string& json_str);
        //int async_kafka_prod(const std::string& json_str);

    private:
        huawei_dialout::gRPCDataservice::AsyncService *huawei_service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext server_ctx;
        huawei_dialout::serviceArgs huawei_stream;
        grpc::ServerAsyncReaderWriter<huawei_dialout::serviceArgs,
                                    huawei_dialout::serviceArgs> huawei_resp;
    };
};

#endif
