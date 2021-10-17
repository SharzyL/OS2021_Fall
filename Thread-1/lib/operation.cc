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

namespace proj1 {
Worker::Worker(EmbeddingHolder &users, EmbeddingHolder &items): users(users), items(items) {
    lock_list.reserve(users.get_n_embeddings());
    for (int i = 0; i < users.get_n_embeddings(); i++) {
        lock_list.emplace_back(new std::shared_mutex);
    }
}

void Worker::op_init_emb(const PayloadType &payload) {
    // read items[item_idx...]
    // write new user
    int length = users.get_emb_length();
    auto* new_user = new Embedding(length);
    int user_idx = users.append(new_user);
    std::vector<std::thread> jobs;
    EmbeddingGradient g_sum(users.get_emb_length());
    std::mutex gradient_write_lock;
    for (int item_index: payload) {
        jobs.emplace_back([&, item_index]() {
            Embedding* item_emb = items.get_embedding(item_index);
            EmbeddingGradient* gradient = cold_start(new_user, item_emb);
            std::unique_lock lock(gradient_write_lock);
            g_sum = g_sum + gradient;
        });
    }
    for (auto &job : jobs) job.join();
    users.update_embedding(user_idx, &g_sum, 0.01);
}

void Worker::op_update_emb(int user_idx, int item_idx, int label) {
    // read items[item_idx], users[user_idx]
    // write users[user_idx]
    Embedding* user = users.get_embedding(user_idx);
    Embedding* item = items.get_embedding(item_idx);
    EmbeddingGradient* user_gradient = calc_gradient(user, item, label);
    users.update_embedding(user_idx, user_gradient, 0.01);
    EmbeddingGradient* item_gradient = calc_gradient(item, user, label);
    items.update_embedding(item_idx, item_gradient, 0.001);
}

Embedding* Worker::op_recommend(const PayloadType &payload) const {
    // read items[item_idx...], users[user_idx]
    int user_idx = payload[0];
    Embedding* user = users.get_embedding(user_idx);
    std::vector<Embedding*> item_pool;
    for (unsigned int i = 1; i < payload.size(); ++i) {
        int item_idx = payload[i];
        item_pool.push_back(items.get_embedding(item_idx));
    }
    Embedding* recommendation = recommend(user, item_pool);
    return recommendation;
}

void Worker::work(Instructions &instructions) {
    // TODO: item update atomicity
    // TODO: recommend output atomicity
    // TODO: update item as fast as possible

    for (const proj1::Instruction& inst: instructions) {
        if (inst.order == proj1::INIT_EMB) {
            lock_list.emplace_back(new std::shared_mutex);
        }
    }

    auto newest_user_idx = users.get_n_embeddings();
    for (const proj1::Instruction& inst: instructions) {
        if (inst.order == proj1::INIT_EMB) {
            jobs_list.emplace_back([=, &inst]() {
                std::unique_lock<std::shared_mutex> lock(*lock_list[newest_user_idx]);
                LOG(INFO) << fmt::format("init {}", newest_user_idx);
                op_init_emb(inst.payloads);
                LOG(INFO) << fmt::format("init {} end", newest_user_idx);
            });
            newest_user_idx++;
        }
    }

    int cur_epoch = 0;
    for (proj1::Instruction& inst: instructions) {
        if (inst.order == proj1::UPDATE_EMB) {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];
            if (inst.payloads.size() > 3 && inst.payloads[3] > cur_epoch) {
                LOG(INFO) << fmt::format("waiting for epoch {}", cur_epoch);
                for (auto &job : update_jobs_list) job.join();
                update_jobs_list.clear();
                LOG(INFO) << fmt::format("waiting for epoch {} end", cur_epoch);
                cur_epoch = inst.payloads[3];
            }
            update_jobs_list.emplace_back([=]() {
                std::unique_lock<std::shared_mutex> lock(*lock_list[user_idx]);
                LOG(INFO) << fmt::format("update {}", user_idx);
                op_update_emb(user_idx, item_idx, label);
                LOG(INFO) << fmt::format("update {} end", user_idx);
            });
        } else if (inst.order == proj1::RECOMMEND) {
            int user_idx = inst.payloads[0];
            jobs_list.emplace_back([=]() {
                std::shared_lock<std::shared_mutex> lock(*lock_list[user_idx]);
                LOG(INFO) << fmt::format("recommend {}", user_idx);
                op_recommend(inst.payloads)->write_to_stdout();
                LOG(INFO) << fmt::format("recommend {} end", user_idx);
            });
        }
    }

    for (auto &job : update_jobs_list) {
        job.join();
    }
    for (auto &job : jobs_list) {
        job.join();
    }
};
} // namespace proj1
