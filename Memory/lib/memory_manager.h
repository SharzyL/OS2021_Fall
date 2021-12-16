#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <bitset>
#include <cassert>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "array_list.h"
#include "evict_mgr.h"

constexpr int PageSize = 1024;

namespace proj3 {

class ArrayList; // placeholder for cyclic dependency

class PageFrame {
public:
    PageFrame(int *mem);
    int &operator[](unsigned long);
    void WriteDisk(const std::string &);
    void ReadDisk(const std::string &);
    void Clear();

private:
    int *mem;
};

class PageInfo {
public:
    PageInfo();
    void SetInfo(int, int);
    void ClearInfo();
    int GetHolder() const;
    int GetVid() const;

private:
    int holder;          // page holder id (array_id)
    int virtual_page_id; // page virtual #
};

class MemoryManager {
public:
    // you should not modify the public interfaces used in tests
    enum EvictAlg { EVICT_CLOCK_ALG, EVICT_FIFO_ALG };
    MemoryManager(int, EvictAlg alg = EVICT_CLOCK_ALG);
    int ReadPage(int arr_id, int vid, int offset);
    void WritePage(int arr_id, int vid, int offset, int value);
    ArrayList *Allocate(int size);
    void Release(ArrayList *);
    MemoryManager(const MemoryManager &) = delete;
    MemoryManager &operator=(const MemoryManager &) = delete;

    ~MemoryManager();

    long long stat_num_access = 0;
    long long stat_num_miss = 0;

private:
    using ulock = std::unique_lock<std::mutex>;

    enum PageTableVal { PAGE_ON_DISK = -1, PAGE_UNALLOCATED = -2 };

    int mma_sz;

    int *underlying_mem;
    std::vector<PageFrame> phy_pages;
    std::vector<PageInfo> phy_pages_info;
    std::map<int, std::vector<int>> page_table; // (array_list_id, virt_page_num) -> phy_page_num

    int next_array_id = 0;

    AllocMgr free_page_mgr;

    std::unique_ptr<EvictMgr> evict_mgr;

    void page_in(int array_id, int vid, int phy_id, ulock &lk);
    void page_out(int phy_id, ulock &lk);

    int allocate_one_page(int arr_id, int vid, ulock &lk); // return phy id of new allocated page
    int replace_one_page(int arr_id, int vid, ulock &lk);  // return phy id of new allocated page
    int locate_page(int arr_id, int vid, ulock &lk);       // return phy id of loaded page

    std::string build_page_file_name(int array_id, int vid);

    std::mutex op_mtx;
    std::mutex pending_set_mtx;
    std::condition_variable op_cv;
    std::set<int> pending_phy_pages;
    std::set<std::pair<int, int>> pending_virt_pages;

    void lock_page(int arr_id, int vid, int phy_page, ulock &lk);
    void unlock_page(int arr_id, int vid, int phy_page, ulock &lk);
};

} // namespace proj3

#endif
