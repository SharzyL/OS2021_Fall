#include <stdexcept>

#include "mma_client.h"
#include "const.h"

namespace proj4 {

MmaClient::MmaClient(std::shared_ptr<grpc::Channel> channel) : stub_(MMA::NewStub(channel)) {}

int MmaClient::ReadPage(int arr_id, int vid, int offset) {
    ReadPageRequest req;
    ReadPageResponse response;
    req.set_arr_id(arr_id);
    req.set_offset(offset);
    req.set_vid(vid);
    grpc::ClientContext ctx;
    grpc::Status status = stub_->ReadPage(&ctx, req, &response);
    // Act upon its status.
    if (status.ok()) {
        return response.data();
    } else {
        throw std::runtime_error("rpc error on ReadPage");
    }
}

void MmaClient::WritePage(int arr_id, int vid, int offset, int value) {
    WritePageRequest req;
    WritePageResponse response;
    req.set_arr_id(arr_id);
    req.set_offset(offset);
    req.set_vid(vid);
    req.set_value(value);
    grpc::ClientContext ctx;
    grpc::Status status = stub_->WritePage(&ctx, req, &response);
    // Act upon its status.
    if (status.ok()) {

    } else {
        throw std::runtime_error("rpc error on ReadPage");
    }
}

ArrayList *MmaClient::Allocate(int size) {
    AllocateRequest req;
    AllocateResponse response;
    req.set_size(size);
    grpc::ClientContext ctx;
    grpc::Status status = stub_->Allocate(&ctx, req, &response);
    // Act upon its status.
    if (status.ok()) {
        return new ArrayList(this, size, response.arr_id());
    } else {
        throw std::runtime_error("rpc error on ReadPage");
    }
}

void MmaClient::Free(ArrayList *arr) {
    FreeRequest req;
    FreeResponse response;
    req.set_arr_id(arr->array_id);
    grpc::ClientContext ctx;
    grpc::Status status = stub_->Free(&ctx, req, &response);
    // Act upon its status.
    if (status.ok()) {

    } else {
        throw std::runtime_error("rpc error on ReadPage");
    }
}

}