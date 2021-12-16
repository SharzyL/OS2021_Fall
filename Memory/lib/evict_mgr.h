//
// Created by sharzy on 12/13/21.
//

#ifndef MEMORY_EVICT_MGR_H
#define MEMORY_EVICT_MGR_H

#include <list>
#include <queue>
#include <vector>

#include <fmt/core.h>
#include <glog/logging.h>

class AllocMgr {
public:
    AllocMgr() = default;
    void Free(int idx) { free_queue.push(idx); };
    int Alloc() {
        if (free_queue.empty()) {
            return -1;
        } else {
            int front = free_queue.front();
            free_queue.pop();
            return front;
        }
    }
    virtual ~AllocMgr() = default;

private:
    std::queue<int> free_queue{};
};

class EvictMgr {
public:
    explicit EvictMgr(int size) : size(size){};
    virtual void Load(int idx) = 0;
    virtual void Access(int idx) = 0;
    virtual int Evict() = 0;
    virtual void Free(int idx) = 0;
    int size;
};

class FifoEvictMgr : public EvictMgr {
public:
    explicit FifoEvictMgr(int size) : EvictMgr(size) {}
    void Load(int idx) override { access_queue.push_back(idx); }
    void Access(int idx) override {}
    int Evict() override {
        int front = access_queue.front();
        LOG(WARNING) << fmt::format("fifo evict {}", front);
        access_queue.pop_front();
        return front;
    }
    void Free(int idx) override { access_queue.remove(idx); }

private:
    std::list<int> access_queue;
};

class ClockEvictMgr : public EvictMgr {
public:
    explicit ClockEvictMgr(int size) : EvictMgr(size), is_visited_recently(size), pointer(0) {}
    void Load(int idx) override { is_visited_recently[idx] = false; }
    void Access(int idx) override { is_visited_recently[idx] = true; }
    int Evict() override {
        while (is_visited_recently[pointer]) {
            is_visited_recently[pointer] = false;
            pointer = (pointer + 1) % size;
        }
        LOG(WARNING) << fmt::format("clock evict {}", pointer);
        return pointer;
    }
    void Free(int idx) override { is_visited_recently[idx] = false; }

private:
    std::vector<int> is_visited_recently;
    int pointer;
};

#endif // MEMORY_EVICT_MGR_H
