#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <bitset>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
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

class AllocMgr {
public:
    AllocMgr();
    void Free(int idx);
    int Alloc();

private:
    std::queue<int> free_queue{};
};

class MemoryManager {
public:
    // you should not modify the public interfaces used in tests
    explicit MemoryManager(int);
    int ReadPage(int array_id, int virtual_page_id, int offset);
    void WritePage(int array_id, int virtual_page_id, int offset, int value);
    ArrayList *Allocate(int size);
    void Release(ArrayList *);
    MemoryManager(const MemoryManager &) = delete;
    MemoryManager &operator=(const MemoryManager &) = delete;

    ~MemoryManager();

private:
    enum PageTableVal { PAGE_ON_DISK = -1, PAGE_UNALLOCATED = -2 };

    int mma_sz;

    int *underlying_mem;
    std::vector<PageFrame> phy_pages;
    std::vector<PageInfo> phy_pages_info;
    std::map<int, std::vector<int>> page_table; // (array_list_id, virt_page_num) -> phy_page_num
    std::map<int, ArrayList> array_list_map;

    int next_array_id = 0;

    AllocMgr free_page_mgr;

    std::unique_ptr<EvictMgr> evict_mgr;

    void page_in(int array_id, int vid, int phy_id);
    void page_out(int phy_id);

    int allocate_one_page(int arr_id, int vid); // return phy id of new allocated page
    int replace_one_page(int arr_id, int vid);  // return phy id of new allocated page
    int locate_page(int arr_id, int vid);       // return phy id of loaded page

    std::string build_page_file_name(int array_id, int vid);
};

} // namespace proj3

#endif
