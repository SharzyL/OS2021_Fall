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
    using unique_lock = std::unique_lock<std::shared_mutex>;
    using shared_lock = std::shared_lock<std::shared_mutex>;
    EmbeddingHolder &users;
    EmbeddingHolder &items;

    Worker(EmbeddingHolder &users, EmbeddingHolder &items);
    void op_init_emb(int user_idx, const std::vector<int> &item_idx_list);
    void op_update_emb(int user_idx, int item_idx, int label);
    void op_recommend(int user_idx, const std::vector<int> &item_idx_list);
    void wait_update_jobs();
    void work(Instructions &instructions);
private:
    std::vector<std::unique_ptr<std::shared_mutex>> user_locks_list;
    std::vector<std::unique_ptr<std::shared_mutex>> item_locks_list;
    std::vector<std::thread> jobs_list;
    std::vector<std::thread> update_jobs_list;
    std::mutex printer_lock;
};

} // namespace proj
#endif //THREAD_1_OPERATION_H
