//
// Created by sharzy on 10/10/21.
//

#ifndef THREAD_1_OPERATION_H
#define THREAD_1_OPERATION_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <map>
#include <variant>

#include "model.h"
#include "embedding.h"
#include "instruction.h"

namespace proj1 {

void run_one_instruction(const Instruction &inst, EmbeddingHolder &users, EmbeddingHolder &items, EmbeddingHolder* recommendations=nullptr);

class Worker {
public:
    using unique_lock = std::unique_lock<std::shared_mutex>;
    using shared_lock = std::shared_lock<std::shared_mutex>;
    EmbeddingHolder &users;
    EmbeddingHolder &items;
    const Instructions &instructions;

    Worker(EmbeddingHolder &users, EmbeddingHolder &items, const Instructions &instructions);
    void work();

protected:
    virtual void output_recommendation(const Embedding &recommendation);
    void op_init_emb(int user_idx, const std::vector<int> &item_idx_list);
    void op_update_emb(int user_idx, int item_idx, int label);
    void op_recommend(int user_idx, const std::vector<int> &item_idx_list);
    std::vector<std::unique_ptr<std::shared_mutex>> user_locks_list;
    std::vector<std::unique_ptr<std::shared_mutex>> item_locks_list;
    std::mutex printer_lock;

    struct InitTask {
        int user_idx;
        std::vector<int> item_idx_list;
    };
    struct UpdateTask {
        int user_idx;
        int item_idx;
        int label;
    };
    struct RecommendTask {
        int user_idx;
        std::vector<int> item_idx_list;
    };
    using Task = std::variant<InitTask, UpdateTask, RecommendTask>;

    std::vector<Task> normal_tasks;
    std::map<int, std::vector<Task>> tasks_in_epoch;

    std::vector<std::thread> jobs_list;
    std::vector<std::thread> epoch_jobs_list;

    void execute_task(const Task &t);
};

} // namespace proj
#endif //THREAD_1_OPERATION_H
