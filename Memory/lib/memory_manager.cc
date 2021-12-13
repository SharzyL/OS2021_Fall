#include <fstream>
#include <stdexcept>
#include <string>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <glog/logging.h>

#include "memory_manager.h"

#include "array_list.h"

namespace proj3 {

PageFrame::PageFrame(int *mem) : mem(mem) {}

int &PageFrame::operator[](unsigned long idx) { return mem[idx]; }

void PageFrame::WriteDisk(const std::string &filename) {
    std::ofstream os(filename);
    os.write(reinterpret_cast<char *>(mem), PageSize * sizeof(int));
}

void PageFrame::ReadDisk(const std::string &filename) {
    std::ifstream is(filename);
    is.read(reinterpret_cast<char *>(mem), PageSize * sizeof(int));
}

void PageFrame::Clear() { std::memset(mem, 0, PageSize * sizeof(int)); }

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

MemoryManager::MemoryManager(int size) : mma_sz(size), underlying_mem(new int[size * PageSize]) {
    phy_pages.reserve(size);

    char *evict_alg_str = std::getenv("EVICT_ALG");
    if (evict_alg_str != nullptr && std::string(evict_alg_str) == "FIFO") {
        evict_mgr = std::unique_ptr<EvictMgr>(new FifoEvictMgr(size));
    } else {
        evict_mgr = std::unique_ptr<EvictMgr>(new ClockEvictMgr(size));
    }
    page_info_list.reserve(size);
    for (int i = 0; i < size; i++) {
        phy_pages.emplace_back(underlying_mem + i * PageSize);
        page_info_list.emplace_back();
        free_page_mgr.Free(i);
    }
}

MemoryManager::~MemoryManager() { delete[] underlying_mem; }

void MemoryManager::page_out(int phy_page) {
    auto &page_info = page_info_list[phy_page];
    page_table[page_info.GetHolder()][page_info.GetVid()] = -1;
    phy_pages[phy_page].WriteDisk(build_page_file_name(page_info.GetHolder(), page_info.GetVid()));

    LOG(INFO) << fmt::format("page out [{}, {}] on {}", page_info.GetHolder(), page_info.GetVid(), phy_page);
}

void MemoryManager::page_in(int arr_id, int vid, int phy_page) {
    phy_pages[phy_page].ReadDisk(build_page_file_name(arr_id, vid));
    page_table[arr_id][vid] = phy_page;
    page_info_list[phy_page].SetInfo(arr_id, vid);

    LOG(INFO) << fmt::format("page in [{}, {}] on {}", arr_id, vid, phy_page);
}

int MemoryManager::allocate_one_page(int arr_id, int vid) {
    int phy_page = free_page_mgr.Alloc();
    if (phy_page < 0) { // not found
        phy_page = evict_mgr->Evict();
        page_out(phy_page);
    }
    phy_pages[phy_page].Clear(); // because weird TA request the memory to be initialized
    page_table[arr_id][vid] = phy_page;
    page_info_list[phy_page].SetInfo(arr_id, vid);
    evict_mgr->Load(phy_page);

    LOG(INFO) << fmt::format("allocate page {} for [{}, {}]", phy_page, arr_id, vid);
    return phy_page;
}

int MemoryManager::replace_one_page(int arr_id, int vid) {
    int phy_page = free_page_mgr.Alloc(); // find free memory to swap in
    if (phy_page < 0) {                   // swap out a page if memory is full
        phy_page = evict_mgr->Evict();
        page_out(phy_page);
    }
    evict_mgr->Load(phy_page);
    page_in(arr_id, vid, phy_page);
    return phy_page;
}

int MemoryManager::locate_page(int arr_id, int vid) {
    auto &page_table_dir = page_table.at(arr_id);
    assert(0 <= vid && vid < (int)page_table_dir.size());

    int phy_page = page_table_dir[vid];
    if (phy_page == PAGE_UNALLOCATED) { // if corresponding page not allocated
        phy_page = allocate_one_page(arr_id, vid);
    } else if (phy_page == PAGE_ON_DISK) { // if the page is swapped out
        phy_page = replace_one_page(arr_id, vid);
    } else { // page is in memory
        // do nothing
    }
    evict_mgr->Access(phy_page);
    LOG(INFO) << fmt::format("resolve [{}, {}] = {}", arr_id, vid, phy_page);
    return phy_page;
}

int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset) {
    int phy_page = locate_page(array_id, virtual_page_id);
    int val = phy_pages[phy_page][offset];
    LOG(INFO) << fmt::format("read [{}, {}][{}] = {}", array_id, virtual_page_id, offset, val);
    return val;
}

void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value) {
    int phy_page = locate_page(array_id, virtual_page_id);
    LOG(INFO) << fmt::format("write [{}, {}][{}] = {}", array_id, virtual_page_id, offset, value);
    phy_pages[phy_page][offset] = value;
}

ArrayList *MemoryManager::Allocate(int sz) {
    int pages_needed = ((int)sz + PageSize - 1) / PageSize;
    if (pages_needed > mma_sz) {
        throw std::runtime_error("not enough mem to allocate");
    }
    int arr_id = next_array_id;
    LOG(INFO) << fmt::format("request {} bytes for app {}", sz, arr_id);
    next_array_id++;
    auto &new_page_table = page_table[arr_id]; // init an empty page table
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
            free_page_mgr.Free(phy_page);
            evict_mgr->Free(phy_page);
            page_info_list[phy_page].ClearInfo();
        } else if (phy_page == PAGE_ON_DISK) {
            // TODO: clean swap file
        }
    }
    page_table.erase(arr->array_id);
}

std::string MemoryManager::build_page_file_name(int array_id, int vid) {
    std::string name = "/tmp/page_" + std::to_string(array_id) + "_" + std::to_string(vid);
    return name;
}

AllocMgr::AllocMgr() = default;

void AllocMgr::Free(int idx) { free_queue.push(idx); }

int AllocMgr::Alloc() {
    if (free_queue.empty()) {
        return -1;
    } else {
        int front = free_queue.front();
        free_queue.pop();
        return front;
    }
}

} // namespace proj3