#include <iostream>
#include <grpcpp/grpcpp.h>
#include "mdt_dialout_core.h"
#include "mdt_dialout_cisco.grpc.pb.h"


ServerImpl::~ServerImpl()
{
    server_->Shutdown();
    cq_->Shutdown();
}

void ServerImpl::Run()
{
    std::string server_addr("0.0.0.0:10000");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    //std::cout << "MDT Server listening on " << server_addr << std::endl;

    ServerImpl::HandleRpcs();
}

void ServerImpl::HandleRpcs()
{
    new ServerImpl::CallData(&service_, cq_.get(), num_clients_++);
    void *tag;
    bool ok;
    while (true) {
        GPR_ASSERT(cq_->Next(&tag, &ok));
        if(!ok) {
            static_cast<CallData*>(tag)->Stop();
            continue;
        }
        static_cast<CallData*>(tag)->Proceed();
    }
}

ServerImpl::CallData::CallData(mdt_dialout::gRPCMdtDialout::AsyncService *service,
                                ServerCompletionQueue *cq) : service_(service),
                                                                cq_(cq),
                                                                responder_(&ctx_),
                                                                status_(CREATE)
{
    ServerImpl::CallData::Proceed();
}

void ServerImpl::CallData::Proceed()
{
    if (status_ == CREATE) {
        service_->mdt_dialout::RequestMdtDialout(&ctx_, &responder_, cq_, cq_, this);
        status_ = PROCESS;
    } else if (status_ == PROCESS) {
        new CallData(service_, cq_, client_id_+1);
        responder_.Read(&msg_, this);
        std::cout << msg_.data() << std::endl;
    } else {
        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}

void ServerImpl::CallData::Stop()
{
    std::cerr << "Finishing up client " << std::endl;
    status_ = FINISH;
}
