/*
 * From the proto file:
 *
 * Package => Service => RPC => Message => Msg-Attributes
 *
 * mdt_dialout => gRPCMdtDialout => MdtDialout => MdtDialoutArgs => Args
 *
 */


#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include "mdt_dialout_cisco.grpc.pb.h"


using grpc::Server;
using grpc::ServerAsyncReaderWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using grpc::WriteOptions;
using mdt_dialout::MdtDialoutArgs;
using mdt_dialout::gRPCMdtDialout;

class ServerImpl final {
 public:
  ~ServerImpl() // OK
  {
    server_->Shutdown();
    cq_->Shutdown();
  }

  void Run() // OK
  {
    std::string server_addr("0.0.0.0:10000");
    ServerBuilder builder;
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    std::cout << "MDT Server listening on " << server_addr << std::endl;

    HandleRpcs();
  }

 private:
  std::unique_ptr<ServerCompletionQueue> cq_;
  mdt_dialout::gRPCMdtDialout::AsyncService service_;
  std::unique_ptr<Server> server_;

  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    CallData(mdt_dialout::gRPCMdtDialout::AsyncService *service,
             ServerCompletionQueue *cq) : service_(service),
                                          cq_(cq),
                                          responder_(&ctx_),
                                          status_(CREATE),
                                          times_(0)
    {
        Proceed();
    }

    void Proceed()
    {
      if (status_ == CREATE) {
        std::cout << "Status: " << status_ << std::endl;
        status_ = PROCESS;
        service_->RequestMdtDialout(&ctx_, &responder_, cq_, cq_, this);
      } else if (status_ == PROCESS) {
          if (times_ == 0) {
            std::cout << "Status: " << status_ << std::endl;
            new CallData(service_, cq_);
          }
          if (times_++ >= 3) {
            status_ = FINISH;
            responder_.Finish(Status::OK, this);
          } else {
            responder_.Read(&msg_, this);
            std::cout << "ReqId: " << msg_.reqid() << std::endl;
            std::cout << "data: " << msg_.data() << std::endl;
            std::cout << "totalSize: " << msg_.totalsize() << std::endl;
            //responder_.Write(msg_, this);
          }
      } else {
        std::cout << "Status: " << status_ << std::endl;
        GPR_ASSERT(status_ == FINISH);
        delete this;
      }
    }

   private:
    mdt_dialout::gRPCMdtDialout::AsyncService *service_;
    ServerCompletionQueue *cq_;
    ServerContext ctx_;

    mdt_dialout::MdtDialoutArgs msg_;
    //mdt_dialout::MdtDialoutArgs reply_;

    ServerAsyncReaderWriter<MdtDialoutArgs, MdtDialoutArgs> responder_;

    size_t times_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
  };

  // threads if needed.
  void HandleRpcs()
  {
    size_t counter = 0;
    new CallData(&service_, cq_.get());
    void *tag;
    bool ok;
    while (true) {
      std::cout << "Counter: " << counter << std::endl;
      GPR_ASSERT(cq_->Next(&tag, &ok));
      //std::cout << "CQ: " << cq_.get() << std::endl;
      GPR_ASSERT(ok);
      static_cast<CallData *>(tag)->Proceed();
      counter++;
    }
  }
};


int main(int argc, char **argv)
{
  ServerImpl server;
  server.Run();

  return 0;
}
