#include "mma_client.h"
#include "const.h"

proj4::MmaClient::MmaClient(std::shared_ptr<grpc::Channel> channel) {}

int proj4::MmaClient::ReadPage(int arr_id, int vid, int offset) { return 0; }

void proj4::MmaClient::WritePage(int arr_id, int vid, int offset, int value) {}

proj4::ArrayList *proj4::MmaClient::Allocate(int size) { return nullptr; }

void proj4::MmaClient::Free(proj4::ArrayList *) {}
