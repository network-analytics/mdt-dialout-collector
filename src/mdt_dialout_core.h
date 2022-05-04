#ifndef _SRV_H_
#define _SRV_H_

#include <iostream>
#include <grpcpp/grpcpp.h>
//#include <json/json.h>
#include "mdt_dialout.grpc.pb.h"

/**
 * Prefix each Class-name or Method-name with MdtDialout
 * should make the code more readable
 */
class Srv final {
public:
    ~Srv();
    void Bind(std::string srv_addr);

private:
    mdt_dialout::gRPCMdtDialout::AsyncService service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::unique_ptr<grpc::Server> server_;
    void FsmCtrl();

    class Stream {
    public:
        Stream(mdt_dialout::gRPCMdtDialout::AsyncService *service,
            grpc::ServerCompletionQueue *cq);
        void Start();
        void Stop();
        int str2json(const std::string& json_str);

    private:
        enum StreamStatus { START, FLOW, END };
        StreamStatus stream_status;
        mdt_dialout::gRPCMdtDialout::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext server_ctx;
        mdt_dialout::MdtDialoutArgs stream;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
                                    mdt_dialout::MdtDialoutArgs> resp;
    };
};

#endif

