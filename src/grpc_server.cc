#include "reader.grpc.pb.h"
#include "reader.pb.h"
#include "csv_processor.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using reader::ReaderService;

ABSL_FLAG(double, result_size, 1.0, "Result size"); 

class ReaderServiceImpl final: public ReaderService::Service {
    //explicit PingServiceImpl() {}
    public:
        ReaderServiceImpl() {
            //_table = table;
        }
        
        Status GetRecord(::grpc::ServerContext* context, const ::reader::ReaderRequest* request, ::reader::ReaderResponse* response) {
            auto field = request->key();
            auto value = request->value();
            float result_size = absl::GetFlag(FLAGS_result_size);
            
            printf("request field %s, value %s\n", field.c_str(), value.c_str());
            auto result = searchLoadedData(field, value);
            printf("result size %d\n", result.size());
            
            response->set_num_record(result.size());

            // limit the number of results
            auto counter = int(result_size * result.size());
            printf("result return size %d\n", counter);
            
            for (auto &record: result) {
                if (counter-- <= 0) {
                    break;
                }

                auto resp_record = response->add_records();
                for (auto &field: record) {
                    auto resp_field = resp_record->add_fields();
                    //printf("%s:%s ", field.first.c_str(), field.second.c_str());
                    resp_field->set_key(field.first);
                    resp_field->set_value(field.second);
                }
                //printf("\n");
            }
            return Status::OK;
        }

        Status GetRecordString(::grpc::ServerContext* context, const ::reader::ReaderRequest* request, ::reader::ReaderResponseString* response) {
            auto field = request->key();
            auto value = request->value();
            float result_size = absl::GetFlag(FLAGS_result_size);
            
            printf("request field %s, value %s\n", field.c_str(), value.c_str());
            auto result = searchLoadedData(field, value);
            printf("result size %d\n", result.size());
            
            // limit the number of results
            auto counter = int(result_size * result.size());
            printf("result return size %d\n", counter);

            response->set_data(std::move(convert_result_to_string(result)));
            return Status::OK;
        }
};

void RunServer() {
    std::string address("0.0.0.0:12423");
    loadAllCSVFiles(directoryPath, headers);

    ReaderServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Starting grpc service" << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    printf("result size %f\n", absl::GetFlag(FLAGS_result_size));

    RunServer();
    return 0;
}