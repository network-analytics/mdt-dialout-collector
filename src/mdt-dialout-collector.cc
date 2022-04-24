#include <grpcpp/grpcpp.h>
#include <string>
#include "mdt_dialout.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;

// mdt_dialout    -> Proto's package
// gRPCMdtDialout -> Proto's Service
//   MdtDialout
// MdtDialoutArgs -> Proto's Message
//   int64 ReqId     = 1;
//   bytes data      = 2;
//   string errors   = 3;
//   int32 totalSize = 4;
using mdt_dialout::MdtDialoutArgs; 
using mdt_dialout::gRPCMdtDialout; 


// Server Implementation
class gRPCMdtDialoutImplementation final : public gRPCMdtDialout::Service {
    
  Status MdtDialout(ServerContext *context, const MdtDialoutArgs *mdt_stream) {
    
    // Obtains the original string from the request
    std::string mdt_data = mdt_stream->data();
    std::cout << "mdt_data: " + mdt_data << std::endl;

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:10000");
  gRPCMdtDialoutImplementation service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which
  // communication with client takes place
  builder.RegisterService(&service);

  // Assembling the server
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "MDT Server listening on port: " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
