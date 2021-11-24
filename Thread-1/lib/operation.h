//
// Created by sharzy on 10/10/21.
//

#ifndef THREAD_1_OPERATION_H
#define THREAD_1_OPERATION_H

#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <variant>

#include "embedding.h"
#include "instruction.h"
#include "model.h"

namespace proj1 {

void run_one_instruction(const Instruction &inst, EmbeddingHolder &users, EmbeddingHolder &items,
                         EmbeddingHolder *recommendations = nullptr);

class Worker {
public:
    EmbeddingHolder &users;
    EmbeddingHolder &items;
    const Instructions &instructions;

    bool work_inplace;

    Worker(EmbeddingHolder &users, EmbeddingHolder &items, const Instructions &instructions, bool work_inplace=false);
    void work();

protected:
    using unique_lock = std::unique_lock<std::shared_mutex>;
    using shared_lock = std::shared_lock<std::shared_mutex>;

    virtual void output_recommendation(const Embedding &recommendation);
    void op_init_emb(int user_idx, const std::vector<int> &item_idx_list);
    void op_update_emb(int user_idx, int item_idx, int label);
    void op_recommend(int user_idx, const std::vector<int> &item_idx_list);

    Embedding get_user_emb(int idx);
    Embedding get_item_emb(int idx);
    void update_user_emb(int idx, const EmbeddingGradient &grad, double step);
    void update_item_emb(int idx, const EmbeddingGradient &grad, double step);
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

} // namespace proj1
#endif // THREAD_1_OPERATION_H
