//
// Created by sharzy on 10/10/21.
//

#include <algorithm>
#include <future>

#include "embedding.h"
#include "instruction.h"
#include "model.h"
#include "operation.h"

#include "fmt/core.h"
#include "fmt/ranges.h"
#include "glog/logging.h"

// black magic for std::visit
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

namespace proj1 {

// reference single-thread implementation
void run_one_instruction(const Instruction &inst, EmbeddingHolder &users, EmbeddingHolder &items,
                         EmbeddingHolder *recommendations) {
    switch (inst.order) {
    case INIT_EMB: {
        // We need to init the embedding
        int length = users.get_emb_length();
        int user_idx = users.emplace_back(Embedding(length));
        Embedding &new_user = users[user_idx];
        for (int item_index : inst.payloads) {
            Embedding &item_emb = items[item_index];
            // Call cold start for downstream applications, slow
            EmbeddingGradient gradient = cold_start(new_user, item_emb);
            users.update_embedding(user_idx, gradient, 0.01);
        }
        break;
    }
    case UPDATE_EMB: {
        int user_idx = inst.payloads[0];
        int item_idx = inst.payloads[1];
        int label = inst.payloads[2];
        Embedding &user = users[user_idx];
        Embedding &item = items[item_idx];
        EmbeddingGradient user_gradient = calc_gradient(user, item, label);
        users.update_embedding(user_idx, user_gradient, 0.01);
        EmbeddingGradient item_gradient = calc_gradient(item, user, label);
        items.update_embedding(item_idx, item_gradient, 0.001);
        break;
    }
    case RECOMMEND: {
        int user_idx = inst.payloads[0];
        Embedding &user = users[user_idx];
        std::vector<std::reference_wrapper<Embedding>> item_pool;
        for (unsigned int i = 2; i < inst.payloads.size(); ++i) {
            int item_idx = inst.payloads[i];
            item_pool.emplace_back(items[item_idx]);
        }
        const Embedding &recommendation = recommend(user, item_pool);
        if (recommendations != nullptr) {
            recommendations->emplace_back(recommendation);
        } else {
            recommendation.write_to_stdout();
        }
        break;
    }
    }
}

Worker::Worker(EmbeddingHolder &users, EmbeddingHolder &items, const Instructions &instructions)
    : users(users), items(items), instructions(instructions) {
    std::vector<Task> last_tasks;

    // initialize locks for items
    item_locks_list.reserve(items.get_n_embeddings());
    for (int i = 0; i < items.get_n_embeddings(); i++) {
        item_locks_list.emplace_back(new std::shared_mutex);
    }

    // initialize locks for users
    user_locks_list.reserve(users.get_n_embeddings());
    for (int i = 0; i < users.get_n_embeddings(); i++) {
        user_locks_list.emplace_back(new std::shared_mutex);
    }

    // parse instructions and allocate for epochs
    for (const proj1::Instruction &inst : instructions) {
        if (inst.order == INIT_EMB) {
            user_locks_list.emplace_back(new std::shared_mutex);

            int new_user_idx = users.emplace_back(Embedding(users.get_emb_length())); // append user

            std::vector<int> item_idx_list;
            item_idx_list.reserve(inst.payloads.size() - 1);
            for (auto item_idx : inst.payloads) {
                item_idx_list.push_back(item_idx);
            }
            std::sort(item_idx_list.begin(), item_idx_list.end());
            InitTask t{new_user_idx, std::move(item_idx_list)};
            normal_tasks.emplace_back(std::move(t));

        } else if (inst.order == RECOMMEND) {
            int user_idx = inst.payloads[0];
            int epoch = inst.payloads[1] + 1;
            std::vector<int> item_idx_list;
            item_idx_list.reserve(inst.payloads.size() - 2);
            for (int i = 2; i < inst.payloads.size(); i++) {
                item_idx_list.push_back(inst.payloads[i]);
            }
            std::sort(item_idx_list.begin(), item_idx_list.end());
            RecommendTask t{user_idx, std::move(item_idx_list)};
            tasks_in_epoch[epoch + 1].emplace_back(std::move(t));

        } else if (inst.order == UPDATE_EMB) {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];
            int epoch = inst.payloads.size() > 3 ? inst.payloads[3] : -1;
            UpdateTask t{user_idx, item_idx, label};
            tasks_in_epoch[epoch].emplace_back(t); // trivially copyable, no need to move
        }
    }
}

void Worker::output_recommendation(const Embedding &recommendation) {
    std::unique_lock<std::mutex> print_guard(printer_lock);
    recommendation.write_to_stdout();
}

void Worker::op_init_emb(int user_idx, const std::vector<int> &item_idx_list) {
    LOG(INFO) << fmt::format("init user={} {}", user_idx, item_idx_list);
    unique_lock lock(*user_locks_list[user_idx]);

    const Embedding user_emb = users[user_idx];
    std::vector<std::future<Embedding>> gradient_futures;
    gradient_futures.reserve(item_idx_list.size());
    for (int item_index : item_idx_list) {
        gradient_futures.emplace_back(std::async(std::launch::async, [item_index, this, &user_emb] {
            shared_lock item_lock(*item_locks_list[item_index]);
            const Embedding item_emb = items[item_index]; // read item
            item_lock.unlock();
            return cold_start(user_emb, item_emb); // slow (20)
        }));
    }

    Embedding total_gradient = Embedding(users.get_emb_length()) * 0;
    for (auto &gradient_future : gradient_futures) {
        total_gradient = total_gradient + gradient_future.get();
    }
    users.update_embedding(user_idx, total_gradient, 0.01);

    LOG(INFO) << fmt::format("init user={} {} end", user_idx, item_idx_list);
}

void Worker::op_update_emb(int user_idx, int item_idx, int label) {
    LOG(INFO) << fmt::format("update user={} item={} label={}", user_idx, item_idx, label);
    unique_lock user_lock(*user_locks_list[user_idx]);
    unique_lock item_lock(*item_locks_list[item_idx]);

    const Embedding user = users[user_idx];                                                       // read user
    const Embedding item = items[item_idx];                                                       // read item
    auto user_gradient_future = std::async(std::launch::async, calc_gradient, user, item, label); // slow
    auto item_gradient_future = std::async(std::launch::async, calc_gradient, item, user, label); // slow
    users.update_embedding(user_idx, user_gradient_future.get(), 0.01);                           // write user
    items.update_embedding(item_idx, item_gradient_future.get(), 0.001);                          // write item
    LOG(INFO) << fmt::format("update user={} item={} label={} end", user_idx, item_idx, label);
}

void Worker::op_recommend(int user_idx, const std::vector<int> &item_idx_list) {
    LOG(INFO) << fmt::format("recommend user={} {}", user_idx, item_idx_list);
    shared_lock user_lock(*user_locks_list[user_idx]);
    for (auto item_idx : item_idx_list) {
        (*item_locks_list[item_idx]).lock_shared();
    }

    Embedding &user = users[user_idx]; // read user
    std::vector<std::reference_wrapper<Embedding>> item_pool;
    item_pool.reserve(item_idx_list.size());
    for (auto item_idx : item_idx_list) {
        item_pool.emplace_back(items[item_idx]); // read item
    }
    const Embedding &recommendation = recommend(user, item_pool);
    output_recommendation(recommendation);
    for (auto iter = item_idx_list.rbegin(); iter != item_idx_list.rend(); iter++) {
        (*item_locks_list[*iter]).unlock_shared();
    }
    LOG(INFO) << fmt::format("recommend user={} {} end", user_idx, item_idx_list);
}

void Worker::execute_task(const Task &t) {
    std::visit(overload{
                   [=](const InitTask &t) { op_init_emb(t.user_idx, t.item_idx_list); },
                   [=](const UpdateTask &t) { op_update_emb(t.user_idx, t.item_idx, t.label); },
                   [=](const RecommendTask &t) { op_recommend(t.user_idx, t.item_idx_list); },
               },
               t);
}

void Worker::work() {
    for (const auto &t : normal_tasks) {
        jobs_list.emplace_back([=] { execute_task(t); });
    }
    LOG(INFO) << "put all normal tasks";
    for (const auto &[epoch, tasks] : tasks_in_epoch) {
        LOG(INFO) << fmt::format("push task in epoch {}", epoch);
        for (const auto &t : tasks) {
            epoch_jobs_list.emplace_back([=] { execute_task(t); });
        }
        LOG(INFO) << fmt::format("wait for epoch {}", epoch);
        for (auto &j : epoch_jobs_list) {
            j.join();
        }
        epoch_jobs_list.clear();
        LOG(INFO) << fmt::format("wait for epoch {} end", epoch);
    }
    for (auto &j : jobs_list) {
        j.join();
    }
}
} // namespace proj1
