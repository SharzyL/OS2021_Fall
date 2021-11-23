#ifndef DEADLOCK_LIB_RESOURCE_MANAGER_H_
#define DEADLOCK_LIB_RESOURCE_MANAGER_H_

#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <ostream>
#include <thread>

#include "thread_manager.h"

namespace proj2 {
enum RESOURCE { GPU = 0, MEMORY, DISK, NETWORK };

struct ResourceStatus {
    int CPU = 0;
    int MEMORY = 0;
    int DISK = 0;
    int NETWORK = 0;

    ResourceStatus() = default;

    explicit ResourceStatus(std::map<RESOURCE, int> map)
        : CPU(map[RESOURCE(0)]), MEMORY(map[RESOURCE(1)]), DISK(map[RESOURCE(2)]), NETWORK(map[RESOURCE(3)]) {}

    bool operator<=(const ResourceStatus &other) const {
        return CPU <= other.CPU && MEMORY <= other.MEMORY && DISK <= other.DISK && NETWORK <= other.NETWORK;
    }

    [[nodiscard]] bool isZero() const { return CPU == 0 && MEMORY == 0 && DISK == 0 && NETWORK == 0; }

    void setZero() {
        CPU = 0;
        MEMORY = 0;
        DISK = 0;
        NETWORK = 0;
    }

    [[nodiscard]] int &operator[](RESOURCE kind) {
        switch (kind) {
        case 0:
            return CPU;
        case 1:
            return MEMORY;
        case 2:
            return DISK;
        case 3:
            return NETWORK;
        }
    }

    void operator+=(const ResourceStatus &other) {
        CPU += other.CPU;
        MEMORY += other.MEMORY;
        DISK += other.DISK;
        NETWORK += other.NETWORK;
    }

    void operator-=(const ResourceStatus &other) {
        CPU -= other.CPU;
        MEMORY -= other.MEMORY;
        DISK -= other.DISK;
        NETWORK -= other.NETWORK;
    }

    friend std::ostream &operator<<(std::ostream &os, const ResourceStatus &res) {
        return os << "(" << res.CPU << ", " << res.MEMORY << ", " << res.DISK << ", " << res.NETWORK << ")";
    }
};

class ResourceManager {
public:
    ResourceManager(ThreadManager *t, ResourceStatus init_count)
        : resource_amount(init_count), resource_amount0(init_count), tmgr(t) {}

    void budget_claim(std::map<RESOURCE, int> budget);

    int request(RESOURCE, int amount);

    void release(RESOURCE, int amount);

private:
    ResourceStatus resource_amount;
    ResourceStatus resource_amount0;
    std::map<RESOURCE, std::mutex> resource_mutex;
    std::map<RESOURCE, std::condition_variable> resource_cv;
    ThreadManager *tmgr;

    std::map<std::thread::id, ResourceStatus> all_requests;
    std::map<std::thread::id, ResourceStatus> all_allocations;
    std::mutex request_mutex;
    std::list<std::thread::id> need_run;
};

} // namespace proj2

#endif