#include <fstream>
#include <stdexcept>
#include <string>

#include <glog/logging.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include "memory_manager.h"

#include "array_list.h"

namespace proj3 {
int alg = 1;
PageFrame::PageFrame(int *mem) : mem(mem) {}

int &PageFrame::operator[](unsigned long idx) {
    return mem[idx];
}

void PageFrame::WriteDisk(const std::string &filename) {
    std::ofstream os(filename);
    os.write(reinterpret_cast<char *>(mem), PageSize * sizeof(int));
}

void PageFrame::ReadDisk(const std::string &filename) {
    std::ifstream is(filename);
    is.read(reinterpret_cast<char *>(mem), PageSize * sizeof(int));
}

void PageFrame::Clear() {
    std::memset(mem, 0, PageSize * sizeof(int));
}

PageInfo::PageInfo() : holder(-1), virtual_page_id(-1) {}

void PageInfo::SetInfo(int cur_holder, int cur_vid) {
    holder = cur_holder;
    virtual_page_id = cur_vid;
}

void PageInfo::ClearInfo() {
    holder = -1;
    virtual_page_id = -1;
}

int PageInfo::GetHolder() const { return holder; }

int PageInfo::GetVid() const { return virtual_page_id; }

MemoryManager::MemoryManager(int sz) : mma_sz(sz), underlying_mem(new int[sz * PageSize]) {
    phy_pages.reserve(sz);
    if (alg == 1) {
        phy_page_clock.reserve(sz);
    }
    page_info_list.reserve(sz);
    for (int i = 0; i < sz; i++) {
        phy_pages.emplace_back(underlying_mem + i * PageSize);
        page_info_list.emplace_back();
        free_phy_pages.free(i);
        if (alg == 1) {
            phy_page_clock.emplace_back(0);
        }
    }
}

MemoryManager::~MemoryManager() {
    delete[] underlying_mem;
}

void MemoryManager::PageOut(int physical_page_id) {
    auto &page_info = page_info_list[physical_page_id];
//    LOG(INFO) << fmt::format("page out [{}, {}] on {}", page_info.GetHolder(), page_info.GetVid(), physical_page_id);
    page_table[page_info.GetHolder()][page_info.GetVid()] = -1;
    phy_pages[physical_page_id].WriteDisk(build_page_file_name(page_info.GetHolder(), page_info.GetVid()));
    // change of corresponding page_info is left to the caller
}

void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id) {
//    LOG(INFO) << fmt::format("page in [{}, {}] on {}", array_id, virtual_page_id, physical_page_id);
    phy_pages[physical_page_id].ReadDisk(build_page_file_name(array_id, virtual_page_id));
    page_table[array_id][virtual_page_id] = physical_page_id;
    page_info_list[physical_page_id].SetInfo(array_id, virtual_page_id);
}

int MemoryManager::AllocateOnePage(int arr_id, int vid) {
    int phy_page = free_phy_pages.alloc();
    if (phy_page < 0) { // not found
        phy_page = find_page_to_evict();
        PageOut(phy_page);
    }
//    LOG(INFO) << fmt::format("allocate page {} for [{}, {}]", phy_page, arr_id, vid);
    phy_pages[phy_page].Clear();  // because weird TA request the memory to be initialized
    page_table[arr_id][vid] = phy_page;
    page_info_list[phy_page].SetInfo(arr_id, vid);
    return phy_page;
}

int MemoryManager::LoadPage(int arr_id, int vid) {
    auto &page_table_dir = page_table.at(arr_id);
    assert(0 <= vid && vid < page_table_dir.size());

    int phy_page = page_table_dir[vid];
    if (phy_page == PAGE_UNALLOCATED) {  // if corresponding page not allocated
        phy_page = AllocateOnePage(arr_id, vid);
        if (alg == 1) {
            phy_page_clock[phy_page] = 0;
        }
    } else if (phy_page == PAGE_ON_DISK) {  // if the page is swapped out
        phy_page = free_phy_pages.alloc();   // find free memory to swap in
        if (phy_page < 0) {  // swap out a page if memory is full
            phy_page = find_page_to_evict();
            PageOut(phy_page);
        }
        PageIn(arr_id, vid, phy_page);
    } else {  // page is in memory
        if (alg == 1) {
            phy_page_clock[phy_page] = 1;
        }
    }
//    LOG(INFO) << fmt::format("resolve [{}, {}] = {}", arr_id, vid, phy_page);
    if (alg == 0) {
        phy_page_queue.push_front(phy_page);
    }
    return phy_page;
}

int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset) {
    int phy_page = LoadPage(array_id, virtual_page_id);
    int val = phy_pages[phy_page][offset];
//    LOG(INFO) << fmt::format("read [{}, {}][{}] = {}", array_id, virtual_page_id, offset, val);
    return val;
}

void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value) {
    int phy_page = LoadPage(array_id, virtual_page_id);
//    LOG(INFO) << fmt::format("write [{}, {}][{}] = {}", array_id, virtual_page_id, offset, value);
    phy_pages[phy_page][offset] = value;
}

ArrayList *MemoryManager::Allocate(size_t sz) {
    int pages_needed = ((int) sz + PageSize - 1) / PageSize;
    if (pages_needed > mma_sz) {
        throw std::runtime_error("not enough mem to allocate");
    }
    int arr_id = next_array_id;
//    LOG(INFO) << fmt::format("request {} bytes for app {}", sz, arr_id);
    next_array_id++;
    auto &new_page_table = page_table[arr_id];  // init an empty page table
    new_page_table.reserve(sz);
    for (int i = 0; i < sz; i++) {
        new_page_table.emplace_back(PAGE_UNALLOCATED);
    }
    array_list_map.emplace(std::make_pair(arr_id, ArrayList{sz, this, arr_id}));
    return &array_list_map.at(arr_id);
}

void MemoryManager::Release(ArrayList *arr) {
    next_array_id++;
    auto &page_table_dir = page_table[arr->array_id];
    for (int phy_page : page_table_dir) {
        if (phy_page >= 0) {
            free_phy_pages.free(phy_page);
            page_info_list[phy_page].ClearInfo();
            if (alg == 0) {
                phy_page_queue.remove(phy_page);
            } else {
                phy_page_clock[phy_page] = 0;
            }
        } else {
            // TODO: clean swap file
        }
    }
    page_table.erase(arr->array_id);
}

int MemoryManager::find_page_to_evict() {
    if (alg == 0) {
        int first_in_page = phy_page_queue.back();
        phy_page_queue.pop_back();
        return first_in_page;
    } else if (alg == 1) {
        while (true) {
            if (phy_page_clock[pointer_clock] == 0){
                pointer_clock = (pointer_clock + 1) % this->mma_sz;
                if (pointer_clock >= 1) {
                    return pointer_clock - 1;
                } else {
                    return this->mma_sz - 1;
                }
            } else {
                phy_page_clock[pointer_clock] = 0;
                pointer_clock = (pointer_clock + 1) % this->mma_sz;
            }
        }
    }
    assert(false);
}

std::string MemoryManager::build_page_file_name(int array_id, int vid) {
    std::string name = "/tmp/page_" + std::to_string(array_id) + "_" + std::to_string(vid);
    return name;
}

AllocMgr::AllocMgr() = default;

void AllocMgr::free(int idx) {
    free_queue.push(idx);
}

int AllocMgr::alloc() {
    if (free_queue.empty()) {
        return -1;
    } else {
        int front = free_queue.front();
        free_queue.pop();
        return front;
    }
}

} // namespace proj3