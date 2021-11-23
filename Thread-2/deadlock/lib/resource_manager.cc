#include "resource_manager.h"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <thread>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <glog/logging.h>

namespace proj2 {

int ResourceManager::request(RESOURCE r, int amount) {
    if (amount <= 0)
        return 1;
    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    auto tid = std::this_thread::get_id();
    while (true) {
        if (this->resource_cv[r].wait_for(lk, std::chrono::milliseconds(100), [this, tid] {
                std::unique_lock<std::mutex> request_guard(request_mutex);
                bool is_thread_need_run = std::find(need_run.begin(), need_run.end(), tid) != need_run.end();
                return is_thread_need_run || all_requests.at(tid) <= this->resource_amount;
            })) {
            LOG(INFO) << fmt::format("Thread {} requests {} of amount {}, while now the remainings are {}. "
                                     "Its needs are {}.",
                                     std::this_thread::get_id(), r, amount, this->resource_amount, all_requests[tid]);

            std::unique_lock<std::mutex> request_guard(request_mutex);
            bool is_thread_need_run = std::find(need_run.begin(), need_run.end(), tid) != need_run.end();
            if (is_thread_need_run) {
                break;
            } else {
                if (all_requests.at(tid) <= this->resource_amount) {
                    this->resource_amount -= all_requests.at(tid);
                    need_run.push_back(tid);
                    break;
                } else {
                    continue;
                }
            }
        } else {
            if (tmgr->is_killed(tid)) {
                return -1;
            }
        }
    }
    std::unique_lock<std::mutex> request_guard(request_mutex);
    all_allocations[tid][r] += amount;
    return 0;
}

void ResourceManager::release(RESOURCE r, int amount) {
    if (amount <= 0)
        return;
    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    std::thread::id this_id = std::this_thread::get_id();
    request_mutex.lock();
    all_allocations[this_id][r] -= amount;
    if (all_allocations[this_id].isZero()) {
        need_run.remove(this_id);
        this->resource_amount += all_requests[this_id];
    }
    request_mutex.unlock();
    this->resource_cv[r].notify_all();
}

void ResourceManager::budget_claim(std::map<RESOURCE, int> budget) {
    // This function is called when some workload starts.
    // The workload will eventually consume all resources it claims
    ResourceStatus now(std::move(budget));
    std::thread::id this_id = std::this_thread::get_id();
    request_mutex.lock();
    all_allocations[this_id].setZero();
    if (all_requests.count(this_id) > 0) {
        all_requests[this_id] = now;
    } else {
        all_requests.insert(std::pair<std::thread::id, ResourceStatus>(this_id, now));
    }
    request_mutex.unlock();
}

} // namespace proj2
