#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <map>
#include <vector>
#include <string>
#include <bitset>

#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "array_list.h"

constexpr size_t PageSize = 1024;

namespace proj3 {

class PageFrame {
public:
    PageFrame(int *mem);
    int& operator[] (unsigned long);
    void WriteDisk(const std::string&);
    void ReadDisk(const std::string&);
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
    int holder; //page holder id (array_id)
    int virtual_page_id; // page virtual #
};

class FreeList {
public:
    FreeList(size_t size);
    bool get(size_t idx);
    void set(size_t idx, bool val);
    int first_zero() const;
private:
};

class MemoryManager {
public:
    // you should not modify the public interfaces used in tests
    explicit MemoryManager(size_t);
    int ReadPage(int array_id, int virtual_page_id, int offset);
    void WritePage(int array_id, int virtual_page_id, int offset, int value);
    ArrayList* Allocate(size_t);
    void Release(ArrayList*);
    ~MemoryManager();

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager &operator=(const MemoryManager&) = delete;
private:
    size_t mma_sz;
    int *underlying_mem;
    std::vector<PageFrame> phy_pages;
    std::vector<PageInfo> page_info_list;
    std::map<int, std::map<int, int>> page_table;  // (array_list_id, virt_page_num) -> phy_page_num

    int next_array_id = 0;

    FreeList freelist;

    int find_page_to_evict();

    void PageIn(int array_id, int vid, int phy_id);
    void PageOut(int phy_id);
    std::string build_page_file_name(int array_id, int vid);
};

}  // namespce: proj3

#endif

