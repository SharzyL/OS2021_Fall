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

std::map<std::thread::id, status> all_requests;
std::map<std::thread::id, std::map<proj2::RESOURCE, int>> all_allocations;
std::mutex request_mutex;
std::list<std::thread::id> need_run;

int ResourceManager::request(RESOURCE r, int amount) {
    if (amount <= 0)
        return 1;
    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    while (true) {
        if (this->resource_cv[r].wait_for(lk, std::chrono::milliseconds(100), [this] {
                request_mutex.lock();
                auto this_id = std::this_thread::get_id();
                bool is_thread_need_run =
                    std::find(need_run.begin(), need_run.end(), std::this_thread::get_id()) != need_run.end();
                request_mutex.unlock();
                return is_thread_need_run || (all_requests[this_id].CPU <= this->resource_amount[RESOURCE(0)] &&
                                                 all_requests[this_id].MEMORY <= this->resource_amount[RESOURCE(1)] &&
                                                 all_requests[this_id].DISK <= this->resource_amount[RESOURCE(2)] &&
                                                 all_requests[this_id].NETWORK <= this->resource_amount[RESOURCE(3)]);
            })) {
            LOG(INFO) << fmt::format("Thread {} requests {} of amount {}, while now the remainings are {}, {}, {}, {}. "
                                     "Its needs are {}, {}, {}, {}.",
                                     std::this_thread::get_id(), r, amount, this->resource_amount[RESOURCE(0)],
                                     this->resource_amount[RESOURCE(1)], this->resource_amount[RESOURCE(2)],
                                     this->resource_amount[RESOURCE(3)], all_requests[std::this_thread::get_id()].CPU,
                                     all_requests[std::this_thread::get_id()].MEMORY,
                                     all_requests[std::this_thread::get_id()].DISK,
                                     all_requests[std::this_thread::get_id()].NETWORK);
            request_mutex.lock();
            auto this_id = std::this_thread::get_id();
            std::list<std::thread::id>::iterator it = std::find(need_run.begin(), need_run.end(), this_id);
            if (it != need_run.end()) {
                request_mutex.unlock();
                break;
            } else {
                if (all_requests[this_id].CPU <= this->resource_amount[RESOURCE(0)] &&
                    all_requests[this_id].MEMORY <= this->resource_amount[RESOURCE(1)] &&
                    all_requests[this_id].DISK <= this->resource_amount[RESOURCE(2)] &&
                    all_requests[this_id].NETWORK <= this->resource_amount[RESOURCE(3)]) {
                    this->resource_amount[RESOURCE(0)] -= all_requests[this_id].CPU;
                    this->resource_amount[RESOURCE(1)] -= all_requests[this_id].MEMORY;
                    this->resource_amount[RESOURCE(2)] -= all_requests[this_id].DISK;
                    this->resource_amount[RESOURCE(3)] -= all_requests[this_id].NETWORK;
                    need_run.push_back(this_id);
                    request_mutex.unlock();
                    break;
                } else {
                    request_mutex.unlock();
                    continue;
                }
            }
        } else {
            auto this_id = std::this_thread::get_id();
            if (tmgr->is_killed(this_id)) {
                return -1;
            }
        }
    }
    auto this_id = std::this_thread::get_id();
    request_mutex.lock();
    all_allocations[this_id][RESOURCE(r)] += amount;
    request_mutex.unlock();
    // this->resource_amount[r] -= amount;
    this->resource_mutex[r].unlock();
    return 0;
}

void ResourceManager::release(RESOURCE r, int amount) {
    if (amount <= 0)
        return;
    std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    std::thread::id this_id = std::this_thread::get_id();
    request_mutex.lock();
    all_allocations[this_id][r] -= amount;
    if (all_allocations[this_id][RESOURCE(0)] == 0 && all_allocations[this_id][RESOURCE(1)] == 0 &&
        all_allocations[this_id][RESOURCE(2)] == 0 && all_allocations[this_id][RESOURCE(3)] == 0) {
        need_run.remove(this_id);
        this->resource_amount[RESOURCE(0)] += all_requests[this_id].CPU;
        this->resource_amount[RESOURCE(1)] += all_requests[this_id].MEMORY;
        this->resource_amount[RESOURCE(2)] += all_requests[this_id].DISK;
        this->resource_amount[RESOURCE(3)] += all_requests[this_id].NETWORK;
    }
    request_mutex.unlock();
    // this->resource_amount[r] += amount;
    this->resource_cv[r].notify_all();
}

void ResourceManager::budget_claim(std::map<RESOURCE, int> budget) {
    // This function is called when some workload starts.
    // The workload will eventually consume all resources it claims

    status now;
    now.CPU = budget[RESOURCE(0)];
    now.MEMORY = budget[RESOURCE(1)];
    now.DISK = budget[RESOURCE(2)];
    now.NETWORK = budget[RESOURCE(3)];
    std::thread::id this_id = std::this_thread::get_id();
    request_mutex.lock();
    all_allocations[this_id][RESOURCE(0)] = 0;
    all_allocations[this_id][RESOURCE(1)] = 0;
    all_allocations[this_id][RESOURCE(2)] = 0;
    all_allocations[this_id][RESOURCE(3)] = 0;
    if (all_requests.count(this_id) > 0) {
        all_requests[this_id] = now;
    } else {
        all_requests.insert(std::pair<std::thread::id, status>(this_id, now));
    }
    request_mutex.unlock();
}

} // namespace proj2
