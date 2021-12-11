#include <fstream>
#include <stdexcept>
#include <string>

#include <glog/logging.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include "memory_manager.h"

#include "array_list.h"

namespace proj3 {
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

MemoryManager::MemoryManager(int sz) : mma_sz(sz), underlying_mem(new int[sz * PageSize]), freelist(sz) {
    phy_pages.reserve(sz);
    page_info_list.reserve(sz);
    for (size_t i = 0; i < sz; i++) {
        phy_pages.emplace_back(underlying_mem + i * PageSize);
        page_info_list.emplace_back();
    }
}

MemoryManager::~MemoryManager() {
    delete[] underlying_mem;
}

void MemoryManager::PageOut(int physical_page_id) {
    auto &page_info = page_info_list[physical_page_id];
    LOG(INFO) << fmt::format("page out [{}, {}] on {}", page_info.GetHolder(), page_info.GetVid(), physical_page_id);
    page_table[page_info.GetHolder()][page_info.GetVid()] = -1;
    phy_pages[physical_page_id].WriteDisk(build_page_file_name(page_info.GetHolder(), page_info.GetVid()));
    // change of corresponding page_info is left to the caller
}

void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id) {
    LOG(INFO) << fmt::format("page in [{}, {}] on {}", array_id, virtual_page_id, physical_page_id);
    phy_pages[physical_page_id].ReadDisk(build_page_file_name(array_id, virtual_page_id));
    page_table[array_id][virtual_page_id] = physical_page_id;
    page_info_list[physical_page_id].SetInfo(array_id, virtual_page_id);
}

int MemoryManager::AllocateOnePage(int arr_id, int vid) {
    int phy_page = freelist.first_zero();
    if (phy_page == -1) { // not found
        phy_page = find_page_to_evict();
        PageOut(phy_page);
    }
    LOG(INFO) << fmt::format("allocate page {} for [{}, {}]", phy_page, arr_id, vid);
    freelist.set(phy_page, true);
    phy_pages[phy_page].Clear();  // because weird TA request the memory to be initialized
    page_table[arr_id][vid] = phy_page;
    page_info_list[phy_page].SetInfo(arr_id, vid);
    return phy_page;
}

int MemoryManager::LoadPage(int arr_id, int vid) {
    auto cur_page_table = page_table.at(arr_id);

    auto phy_page_ptr = cur_page_table.find(vid);
    int phy_page;
    if (phy_page_ptr == cur_page_table.end()) {  // if corresponding page not allocated
        phy_page = AllocateOnePage(arr_id, vid);
    } else {
        phy_page = phy_page_ptr->second;
        if (phy_page < 0) {  // if the page is swapped out
            phy_page = find_page_to_evict();
            PageOut(phy_page);
            phy_page_queue.remove(phy_page);
            PageIn(arr_id, vid, phy_page);
        }
        // else page is in memory
    }
    LOG(INFO) << fmt::format("resolve [{}, {}] = {}", arr_id, vid, phy_page);
    phy_page_queue.push_front(phy_page);
    return phy_page;
}

int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset) {
    int phy_page = LoadPage(array_id, virtual_page_id);
    int val = phy_pages[phy_page][offset];
    LOG(INFO) << fmt::format("read [{}, {}][{}] = {}", array_id, virtual_page_id, offset, val);
    return val;
}

void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value) {
    int phy_page = LoadPage(array_id, virtual_page_id);
    LOG(INFO) << fmt::format("write [{}, {}][{}] = {}", array_id, virtual_page_id, offset, value);
    phy_pages[phy_page][offset] = value;
}

ArrayList *MemoryManager::Allocate(size_t sz) {
    int pages_needed = ((int) sz + PageSize - 1) / PageSize;
    if (pages_needed > mma_sz) {
        throw std::runtime_error("not enough mem to allocate");
    }
    int arr_id = next_array_id;
    LOG(INFO) << fmt::format("request {} bytes for app {}", sz, arr_id);
    next_array_id++;
    page_table[arr_id];  // init an empty page table
    array_list_map.emplace(std::make_pair(arr_id, ArrayList{sz, this, arr_id}));
    return &array_list_map.at(arr_id);
}

void MemoryManager::Release(ArrayList *arr) {
    next_array_id++;
    for (auto [i, phy_page] : page_table[arr->array_id]) {
        if (phy_page >= 0) {
            freelist.set(phy_page, false);
            page_info_list[phy_page].ClearInfo();
            phy_page_queue.remove(phy_page);
        } else {
            // TODO: clean swap file
        }
    }
    page_table.erase(arr->array_id);
}

int MemoryManager::find_page_to_evict() {
    int first_in_page = phy_page_queue.back();
    phy_page_queue.pop_back();
    return first_in_page;
}

std::string MemoryManager::build_page_file_name(int array_id, int vid) {
    std::string name = "/tmp/page_" + std::to_string(array_id) + "_" + std::to_string(vid);
    return name;
}

FreeList::FreeList(size_t size): bitset(size), cnt(0) {}

bool FreeList::get(size_t idx) {
    return bitset[idx];
}

void FreeList::set(size_t idx, bool val) {
    cnt -= bitset[idx];
    bitset[idx] = val;
    cnt += val;
}

int FreeList::first_zero() const {
    if (cnt == bitset.size()) return -1;
    for (size_t i = 0; i < bitset.size(); i++) {
        if (!bitset[i]) return (int) i;
    }
    assert(false);  // it should not reach here
}

} // namespace proj3