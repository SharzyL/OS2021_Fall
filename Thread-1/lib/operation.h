//
// Created by sharzy on 10/10/21.
//

#ifndef THREAD_1_OPERATION_H
#define THREAD_1_OPERATION_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "model.h"
#include "embedding.h"
#include "instruction.h"

namespace proj1 {

using PayloadType = decltype(Instruction::payloads);

class Worker {
public:
    EmbeddingHolder &users;
    EmbeddingHolder &items;

    Worker(EmbeddingHolder &users, EmbeddingHolder &items);
    void op_init_emb(const PayloadType &payload);
    void op_update_emb(int user_idx, int item_idx, int label);
    Embedding* op_recommend(const PayloadType &payload) const;
    void work(Instructions &instructions);
private:
    std::vector<std::unique_ptr<std::shared_mutex>> lock_list;
    std::vector<std::thread> jobs_list;
    std::vector<std::thread> update_jobs_list;
};

} // namespace proj
#endif //THREAD_1_OPERATION_H
