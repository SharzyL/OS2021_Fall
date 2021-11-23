#ifndef DEADLOCK_LIB_RESOURCE_MANAGER_H_
#define DEADLOCK_LIB_RESOURCE_MANAGER_H_

#include "thread_manager.h"
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

namespace proj2 {

struct status {
    int CPU = 0;
    int MEMORY = 0;
    int DISK = 0;
    int NETWORK = 0;
};

enum RESOURCE { GPU = 0, MEMORY, DISK, NETWORK };

class ResourceManager {
public:
    ResourceManager(ThreadManager *t, std::map<RESOURCE, int> init_count)
        : resource_amount(init_count), resource_amount0(init_count), tmgr(t) {}
    void budget_claim(std::map<RESOURCE, int> budget);
    int request(RESOURCE, int amount);
    void release(RESOURCE, int amount);

private:
    std::map<RESOURCE, int> resource_amount;
    std::map<RESOURCE, int> resource_amount0;
    std::map<RESOURCE, std::mutex> resource_mutex;
    std::map<RESOURCE, std::condition_variable> resource_cv;
    ThreadManager *tmgr;
};

} // namespace proj2

#endif