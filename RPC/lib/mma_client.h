#ifndef MMA_CLIENT_H
#define MMA_CLIENT_H

#include <cstdlib>
#include <memory>

#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "proto/mma.grpc.pb.h"
#else
#include "mma.grpc.pb.h"
#endif

#include "array_list.h"

namespace proj4 {

class ArrayList;

class MmaClient {
public:
    explicit MmaClient(std::shared_ptr<grpc::Channel> channel);
    int ReadPage(int arr_id, int vid, int offset);
    void WritePage(int arr_id, int vid, int offset, int value);
    ArrayList *Allocate(int size);
    void Free(ArrayList *);
};

} // namespace proj4

#endif