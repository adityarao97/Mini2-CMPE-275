#include "reader.grpc.pb.h"
#include "reader.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using reader::PingService;

class PingServiceImpl final: public PingService::Service {
    //explicit PingServiceImpl() {}

    Status Ping(::grpc::ServerContext* context, const ::reader::PingRequest* request, ::reader::PingResponse* response) {
        response->set_message(request->message());
        return Status::OK;
    }
};

void RunServer() {
    std::string address("0.0.0.0:12423");
    PingServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Starting grpc service" << std::endl;
    server->Wait();
}

int main() {
    RunServer(); 
    return 0;
}