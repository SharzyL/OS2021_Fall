//
// Created by sharzy on 10/10/21.
//

#include <algorithm>

#include "model.h"
#include "embedding.h"
#include "instruction.h"
#include "operation.h"

#include "glog/logging.h"
#include "fmt/core.h"
#include "fmt/ranges.h"
#include <filesystem>

namespace proj1 {

Worker::Worker(EmbeddingHolder &users, EmbeddingHolder &items) : users(users), items(items) {
    user_locks_list.reserve(users.get_n_embeddings());
    for (int i = 0; i < users.get_n_embeddings(); i++) {
        user_locks_list.emplace_back(new std::shared_mutex);
    }
    for (int i = 0; i < items.get_n_embeddings(); i++) {
        item_locks_list.emplace_back(new std::shared_mutex);
    }
}

void Worker::op_init_emb(int user_idx, const std::vector<int> &item_idx_list) {
    unique_lock lock(*user_locks_list[user_idx]);
    for (auto item_idx : item_idx_list) {
        (*item_locks_list[item_idx]).lock_shared();
    }

    auto *new_user = new Embedding(users.get_emb_length());
    users.append(new_user);  // append user

    std::vector<std::thread> jobs;
    EmbeddingGradient g_sum(users.get_emb_length());
    std::mutex gradient_write_lock;

    jobs.reserve(item_idx_list.size());
    for (int item_index: item_idx_list) {
        jobs.emplace_back([&, item_index]() {
            Embedding *item_emb = items.get_embedding(item_index);  // read item
            EmbeddingGradient *gradient = cold_start(new_user, item_emb);  // slow
            {
                std::unique_lock lock(gradient_write_lock);
                g_sum = g_sum + gradient;
            }
        });
    }

    for (auto &job: jobs) job.join();
    users.update_embedding(user_idx, &g_sum, 0.01);  // slow, write user

    for (auto iter = item_idx_list.rbegin(); iter != item_idx_list.rend(); iter++) {
        (*item_locks_list[*iter]).unlock_shared();
    }
}

void Worker::op_update_emb(int user_idx, int item_idx, int label) {
    unique_lock user_lock(*user_locks_list[user_idx]);
    unique_lock item_lock(*item_locks_list[item_idx]);

    Embedding *user = users.get_embedding(user_idx);  // read user
    Embedding *item = items.get_embedding(item_idx);  // read item
    EmbeddingGradient *user_gradient = calc_gradient(user, item, label);  // slow
    users.update_embedding(user_idx, user_gradient, 0.01);  // write user
    EmbeddingGradient *item_gradient = calc_gradient(item, user, label);  // slow
    items.update_embedding(item_idx, item_gradient, 0.001);  // write item
}

void Worker::op_recommend(int user_idx, const std::vector<int> &item_idx_list) {
    shared_lock user_lock(*user_locks_list[user_idx]);
    for (auto item_idx : item_idx_list) {
        (*item_locks_list[item_idx]).lock_shared();
    }

    Embedding *user = users.get_embedding(user_idx);  // read user
    std::vector<Embedding *> item_pool;
    item_pool.reserve(item_idx_list.size());
    for (auto item_idx : item_idx_list) {
        item_pool.push_back(items.get_embedding(item_idx));  // read item
    }
    Embedding *recommendation = recommend(user, item_pool);
    {
        std::unique_lock<std::mutex> print_guard(printer_lock);
        recommendation->write_to_stdout();  // print
    }
    for (auto iter = item_idx_list.rbegin(); iter != item_idx_list.rend(); iter++) {
        (*item_locks_list[*iter]).unlock_shared();
    }
}

void Worker::wait_update_jobs() {
    for (auto &job: update_jobs_list) job.join();
    update_jobs_list.clear();
}

void Worker::work(Instructions &instructions) {
    for (const proj1::Instruction &inst: instructions) {
        if (inst.order == proj1::INIT_EMB) {
            user_locks_list.emplace_back(new std::shared_mutex);
        }
    }

    int newest_user_idx = (int) users.get_n_embeddings();
    for (const proj1::Instruction &inst: instructions) {
        if (inst.order == proj1::INIT_EMB) {
            std::vector<int> item_idx_list;
            item_idx_list.reserve(inst.payloads.size() - 1);
            for (auto item_idx : inst.payloads) {
                item_idx_list.push_back(item_idx);
            }
            jobs_list.emplace_back([=] {
                LOG(INFO) << fmt::format("init {}", newest_user_idx);
                op_init_emb(newest_user_idx, item_idx_list);
                LOG(INFO) << fmt::format("init {} end", newest_user_idx);
            });
            newest_user_idx++;
        }
    }

    int cur_epoch = 0;
    for (proj1::Instruction &inst: instructions) {
        if (inst.order == proj1::UPDATE_EMB) {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];

            if (inst.payloads.size() > 3 && inst.payloads[3] > cur_epoch) {
                LOG(INFO) << fmt::format("waiting for epoch {}", cur_epoch);
                wait_update_jobs();
                LOG(INFO) << fmt::format("waiting for epoch {} end", cur_epoch);
                cur_epoch = inst.payloads[3];
            }

            update_jobs_list.emplace_back([=] {
                LOG(INFO) << fmt::format("update {}", user_idx);
                op_update_emb(user_idx, item_idx, label);
                LOG(INFO) << fmt::format("update {} end", user_idx);
            });

        } else if (inst.order == proj1::RECOMMEND) {
            int user_idx = inst.payloads[0];
            std::vector<int> item_idx_list;
            item_idx_list.reserve(inst.payloads.size() - 1);
            for (int i = 1; i < inst.payloads.size(); i++) {
                item_idx_list.push_back(inst.payloads[i]);
            }

            jobs_list.emplace_back([=] {
                LOG(INFO) << fmt::format("recommend {}", user_idx);
                op_recommend(user_idx, item_idx_list);
                LOG(INFO) << fmt::format("recommend {} end", user_idx);
            });
        }
    }

    for (auto &job: update_jobs_list) {
        job.join();
    }
    for (auto &job: jobs_list) {
        job.join();
    }
};
} // namespace proj1
