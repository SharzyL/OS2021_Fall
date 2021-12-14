#include <filesystem>
#include <fstream>
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
    phy_pages_info.reserve(size);
    for (int i = 0; i < size; i++) {
        phy_pages.emplace_back(underlying_mem + i * PageSize);
        phy_pages_info.emplace_back();
        free_page_mgr.Free(i);
    }
}

MemoryManager::~MemoryManager() {
    delete[] underlying_mem;
    for (auto array_list_ptr: all_array_lists) delete array_list_ptr;
}

void MemoryManager::page_out(int phy_page, ulock &lk) {
    auto &page_info = phy_pages_info[phy_page];
    int ori_arr_id = page_info.GetHolder();
    int ori_vid = page_info.GetVid();

    page_table[ori_arr_id][ori_vid] = PAGE_ON_DISK;

//    lock_page(ori_arr_id, ori_vid, phy_page, lk);
    phy_pages[phy_page].WriteDisk(build_page_file_name(ori_arr_id, ori_vid));
//    unlock_page(ori_arr_id, ori_vid, phy_page, lk);

    LOG(INFO) << fmt::format("page out [{}, {}] on {}", page_info.GetHolder(), page_info.GetVid(), phy_page);
}

void MemoryManager::page_in(int arr_id, int vid, int phy_page, ulock &lk) {
    page_table[arr_id][vid] = phy_page;
    phy_pages_info[phy_page].SetInfo(arr_id, vid);
//    lock_page(arr_id, vid, phy_page, lk);
    phy_pages[phy_page].ReadDisk(build_page_file_name(arr_id, vid));
//    unlock_page(arr_id, vid, phy_page, lk);

    LOG(INFO) << fmt::format("page in [{}, {}] on {}", arr_id, vid, phy_page);
}

int MemoryManager::allocate_one_page(int arr_id, int vid, ulock &lk) {
    int phy_page = free_page_mgr.Alloc();
    if (phy_page < 0) { // not found
        phy_page = evict_mgr->Evict();
        page_out(phy_page, lk);
    }
    phy_pages[phy_page].Clear();
    page_table[arr_id][vid] = phy_page;
    phy_pages_info[phy_page].SetInfo(arr_id, vid);
    evict_mgr->Load(phy_page);

    LOG(INFO) << fmt::format("allocate page {} for [{}, {}]", phy_page, arr_id, vid);
    return phy_page;
}

int MemoryManager::replace_one_page(int arr_id, int vid, ulock &lk) {
    int phy_page = free_page_mgr.Alloc(); // find free memory to swap in
    if (phy_page < 0) {                   // swap out a page if memory is full
        phy_page = evict_mgr->Evict();
        page_out(phy_page, lk);
    }
    evict_mgr->Load(phy_page);
    page_in(arr_id, vid, phy_page, lk);
    return phy_page;
}

int MemoryManager::locate_page(int arr_id, int vid, ulock &lk) {
    auto &page_table_dir = page_table.at(arr_id);
    assert(0 <= vid && vid < (int)page_table_dir.size());

    int phy_page = page_table_dir[vid];
    if (phy_page == PAGE_UNALLOCATED) { // if corresponding page not allocated
        phy_page = allocate_one_page(arr_id, vid, lk);
    } else if (phy_page == PAGE_ON_DISK) { // if the page is swapped out
        phy_page = replace_one_page(arr_id, vid, lk);
    } else { // page is in memory
        // do nothing
    }
    evict_mgr->Access(phy_page);
    LOG(INFO) << fmt::format("resolve [{}, {}] = {}", arr_id, vid, phy_page);
    return phy_page;
}

int MemoryManager::ReadPage(int arr_id, int vid, int offset) {
    assert(page_table.find(arr_id) != page_table.end() && vid < (int) page_table[arr_id].size());

    ulock lk(op_mtx);
    int phy_page = locate_page(arr_id, vid, lk);
    int val = phy_pages[phy_page][offset];
    LOG(INFO) << fmt::format("read [{}, {}][{}] = {}", arr_id, vid, offset, val);
    return val;
}

void MemoryManager::WritePage(int arr_id, int vid, int offset, int value) {
    assert(page_table.find(arr_id) != page_table.end() && vid < (int) page_table[arr_id].size());

    ulock lk(op_mtx);
    int phy_page = locate_page(arr_id, vid, lk);
    LOG(INFO) << fmt::format("write [{}, {}][{}] = {}", arr_id, vid, offset, value);
    phy_pages[phy_page][offset] = value;
}

ArrayList *MemoryManager::Allocate(int sz) {
    ulock lk(op_mtx);
    int pages_needed = ((int)sz + PageSize - 1) / PageSize;
    int arr_id = next_array_id;
    LOG(INFO) << fmt::format("request {} bytes for app {}", sz, arr_id);
    next_array_id++;
    auto &new_page_table = page_table[arr_id]; // init an empty page table
    new_page_table.reserve(pages_needed);
    for (int i = 0; i < pages_needed; i++) {
        new_page_table.emplace_back(PAGE_UNALLOCATED);
    }
    return all_array_lists.emplace_back(new ArrayList(sz, this, arr_id));
}

void MemoryManager::Release(ArrayList *arr) {
    ulock lk(op_mtx);
    assert(page_table.count(arr->array_id) > 0);
    auto &page_table_dir = page_table[arr->array_id];
    for (int vid = 0; vid < (int) page_table_dir.size(); vid++) {
        int phy_page = page_table_dir[vid];
        if (phy_page >= 0) {
            free_page_mgr.Free(phy_page);
            evict_mgr->Free(phy_page);
            phy_pages_info[phy_page].ClearInfo();
        } else if (phy_page == PAGE_ON_DISK) {
            auto page_file_name = build_page_file_name(arr->array_id, vid);
            assert(std::filesystem::exists(page_file_name));
            std::filesystem::remove(page_file_name);
        }
    }
    page_table.erase(arr->array_id);
}

std::string MemoryManager::build_page_file_name(int array_id, int vid) {
    std::string name = "my_pages_" + std::to_string(array_id) + "_" + std::to_string(vid);
    return name;
}

void MemoryManager::lock_page(int arr_id, int vid, int phy_page, MemoryManager::ulock &lk) {
    page_table[arr_id][vid] = PAGE_ON_DISK;
    op_cv.wait(lk, [=] {
        return pending_phy_pages.count(phy_page) == 0 &&
               pending_virt_pages.count({arr_id, vid}) == 0;
    });
    ulock pending_lk(pending_set_mtx);
    pending_phy_pages.emplace(phy_page);
    pending_virt_pages.emplace(arr_id, vid);
}

void MemoryManager::unlock_page(int arr_id, int vid, int phy_page, MemoryManager::ulock &lk) {
    ulock pending_lk(pending_set_mtx);
    pending_phy_pages.erase(phy_page);
    pending_virt_pages.erase({arr_id, vid});
    pending_lk.unlock();
    op_cv.notify_one();
}

} // namespace proj3