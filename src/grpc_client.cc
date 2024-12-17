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

using reader::ReaderService;
using reader::ReaderRequest;
using reader::ReaderResponse;

class ReaderClient {
    public:
        ReaderClient(std::shared_ptr<grpc::Channel> channel): 
            stub(ReaderService::NewStub(channel))
        {}
    
        void Reader(const std::string& key, const std::string& value) {
            grpc::ClientContext context;
            ReaderRequest request;
            request.set_key(key);
            request.set_value(value);

            ReaderResponse response;
            stub->GetRecord(&context, request, &response);
            
            for (int i = 0; i < response.records_size(); i++) {
                auto record = response.records(i);
                std::string output;
                for (int j = 0; j < record.fields_size(); j++) {
                    auto field = record.fields(j);
                    //printf("%s:%s ",field.key().c_str(), field.value().c_str()); 
                }
                //printf("\n");
            }
            printf("response size %d\n", response.num_record());
        }
    private:
        std::unique_ptr<ReaderService::Stub> stub;
};

int main() {
    auto channel = grpc::CreateChannel("0.0.0.0:12423", grpc::InsecureChannelCredentials());
    auto client = ReaderClient(channel);
    std::vector<std::thread> threads;

    for (int i = 0; i < 1; i++) {
        auto fn = [&client]() {
            std::string msg = "hello word";
            client.Reader("location1", "\"BELLFNT2\"");
        };
        threads.push_back(std::thread(fn));
    }

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }
    return 0;
}