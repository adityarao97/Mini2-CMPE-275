#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "reader.grpc.pb.h"

#include <string>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

using reader::PingService;
using reader::PingRequest;
using reader::PingResponse;

class PingClient {
    public:
        PingClient(std::shared_ptr<grpc::Channel> channel): 
            stub(PingService::NewStub(channel))
        {}
    
        void Ping(const std::string& msg) {
            grpc::ClientContext context;
            PingRequest request;
            request.set_message(msg);

            PingResponse response;
            stub->Ping(&context, request, &response);

            printf("ping response %s\n", response.message().c_str());
        }
    private:
        std::unique_ptr<PingService::Stub> stub;
};

int main() {
    auto channel = grpc::CreateChannel("0.0.0.0:12423", grpc::InsecureChannelCredentials());
    auto client = PingClient(channel);
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; i++) {
        auto fn = [&client]() {
            std::string msg = "hello word";
            client.Ping(msg);
        };
        threads.push_back(std::thread(fn));
    }

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }
    return 0;
}