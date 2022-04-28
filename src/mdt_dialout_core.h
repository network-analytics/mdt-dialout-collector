#include <iostream>
#include <grpcpp/grpcpp.h>
#include "mdt_dialout_cisco.grpc.pb.h"


class ServerImpl final {
public:
    ~ServerImpl();
    void Run();

private:
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    mdt_dialout::gRPCMdtDialout::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;

    void HandleRpcs();
    
    class CallData {
    public:
        CallData(mdt_dialout::gRPCMdtDialout::AsyncService *service,
                grpc::ServerCompletionQueue *cq) : service_(service),
                                                    cq_(cq),
                                                    responder_(&ctx_),
                                                    status_(CREATE);
        void Proceed();
        void Stop();

    private:
        mdt_dialout::gRPCMdtDialout::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        grpc::ServerContext ctx_;
        mdt_dialout::MdtDialoutArgs msg_;
        grpc::ServerAsyncReaderWriter<mdt_dialout::MdtDialoutArgs,
                                    mdt_dialout::MdtDialoutArgs> responder_;
        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };
};
