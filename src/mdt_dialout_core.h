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
    void Bind(std::string cisco_srv_socket, std::string huawei_srv_socket);

private:
    mdt_dialout::gRPCMdtDialout::AsyncService cisco_service_;
    huawei_dialout::gRPCDataservice::AsyncService huawei_service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cisco_cq_;
    std::unique_ptr<grpc::ServerCompletionQueue> huawei_cq_;
    std::unique_ptr<grpc::Server> cisco_server_;
    std::unique_ptr<grpc::Server> huawei_server_;
    void CiscoFsmCtrl();
    void HuaweiFsmCtrl();

    class CiscoStream {
    public:
        CiscoStream(mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service,
            grpc::ServerCompletionQueue *cisco_cq);
        void Start();
        void Stop();
        int str2json(const std::string& json_str);
        int async_kafka_prod(const std::string& json_str);

    private:
        enum StreamStatus { START, FLOW, END };
        StreamStatus cisco_stream_status;
        mdt_dialout::gRPCMdtDialout::AsyncService *cisco_service_;
        grpc::ServerCompletionQueue *cisco_cq_;
        grpc::ServerContext cisco_server_ctx;
        mdt_dialout::MdtDialoutArgs cisco_stream;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
                                    mdt_dialout::MdtDialoutArgs> cisco_resp;
    };

    class HuaweiStream {
    public:
        HuaweiStream(huawei_dialout::gRPCDataservice::AsyncService *huawei_service,
            grpc::ServerCompletionQueue *huawei_cq);
        void Start();
        void Stop();
        //int str2json(const std::string& json_str);
        //int async_kafka_prod(const std::string& json_str);

    private:
        enum StreamStatus { START, FLOW, END };
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
