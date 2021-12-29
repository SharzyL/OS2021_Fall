#include <memory>

#include <glog/logging.h>
#include <fmt/core.h>

#include "mma_server.h"
#include "memory_manager.h"

namespace proj4 {

using grpc::Status;
using grpc::ServerContext;

class ServerImpl final : public MMA::Service {
public:
    explicit ServerImpl(int phy_page_num): mma_impl(phy_page_num) {

    }

    Status ReadPage(ServerContext *context, const ReadPageRequest *request, ReadPageResponse *response) override {
        int data = mma_impl.ReadPage(request->arr_id(), request->vid(), request->offset());
        response->set_data(data);
        return Status::OK;
    };

    Status WritePage(ServerContext *context, const WritePageRequest *request, WritePageResponse *response) override {
        mma_impl.WritePage(request->arr_id(), request->vid(), request->offset(), request->value());
        return Status::OK;
    };

    Status Allocate(ServerContext *context, const AllocateRequest *request, AllocateResponse *response) override {
        int arr_id = mma_impl.Allocate(request->size());
        response->set_arr_id(arr_id);
        return Status::OK;
    };

    Status Free(ServerContext *context, const FreeRequest *request, FreeResponse *response) override {
        mma_impl.Release(request->arr_id());
        return Status::OK;
    };
private:
    MemoryManager mma_impl;
};

void RunServerUL(size_t phy_page_num) {
    std::string server_addr("localhost:50051");
    ServerImpl service((int) phy_page_num);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    LOG(WARNING) << fmt::format("server listening on {}", server_addr);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
}

void RunServerL(size_t phy_page_num, size_t max_vir_page_num) {
    std::string server_addr("localhost:50051");
    ServerImpl service((int) phy_page_num);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    LOG(WARNING) << fmt::format("server listening on {}", server_addr);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
}

void ShutdownServer() {

}

}