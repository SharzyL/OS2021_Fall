#include <stdexcept>
#include <fstream>
#include <bitset>

#include "memory_manager.h"

#include "array_list.h"

namespace proj3 {
    PageFrame::PageFrame(int *mem) : mem(mem) {
    }

    int &PageFrame::operator[](unsigned long idx) {
        if (0 <= idx && idx < PageSize) {
            return mem[idx];
        } else {
            throw std::out_of_range("accessing illegal memory");
        }
    }

    void PageFrame::WriteDisk(const std::string &filename) {
        std::ofstream os(filename);
        os.write(reinterpret_cast<char *>(mem), sizeof(mem));
    }

    void PageFrame::ReadDisk(const std::string &filename) {
        std::ifstream is(filename);
        is.read(reinterpret_cast<char *>(mem), sizeof(mem));
    }

    PageInfo::PageInfo(): holder(-1), virtual_page_id(-1)
    {
    }

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

    MemoryManager::MemoryManager(size_t sz) : mma_sz(sz), underlying_mem(new int[sz * PageSize]), freelist(sz) {
        phy_pages.reserve(sz);
        page_info_list.reserve(sz);
        for (int i = 0; i < sz; i++) {
            phy_pages.emplace_back(underlying_mem + i * PageSize);
            page_info_list.emplace_back();
        }
    }

    MemoryManager::~MemoryManager() {
        delete []underlying_mem;
    }

    void MemoryManager::PageOut(int physical_page_id) {
        auto &page_info = page_info_list[physical_page_id];
        page_table[page_info.GetHolder()][page_info.GetVid()] = -1;
        phy_pages[physical_page_id].WriteDisk(build_page_file_name(page_info.GetHolder(), page_info.GetVid()));
        // change of corresponding page_info is left to the caller
    }

    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id) {
        phy_pages[physical_page_id].ReadDisk(build_page_file_name(array_id, virtual_page_id));
        page_table[array_id][virtual_page_id] = physical_page_id;
        page_info_list[physical_page_id].SetInfo(array_id, virtual_page_id);
    }

    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset) {
        int phy_page_id = page_table[array_id][virtual_page_id];
        if (phy_page_id < 0) {
            phy_page_id = find_page_to_evict();
            PageOut(phy_page_id);
            PageIn(array_id, virtual_page_id, phy_page_id);
        }
        return phy_pages[phy_page_id][offset];
    }

    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value) {
        int phy_page_id = page_table[array_id][virtual_page_id];
        if (phy_page_id < 0) {
            phy_page_id = find_page_to_evict();
            PageOut(phy_page_id);
            PageIn(array_id, virtual_page_id, phy_page_id);
        }
        phy_pages[phy_page_id][offset] = value;
    }

    ArrayList *MemoryManager::Allocate(size_t sz) {
        int arr_id = next_array_id;
        next_array_id ++;
        auto &cur_page_table = page_table[arr_id];
        for (int i = 0; i < sz; i++) {
            int next_phy_page = freelist.first_zero();
            if (next_phy_page == -1) {  // not found
                next_phy_page = find_page_to_evict();
                PageOut(next_phy_page);
            }
            freelist.set(next_phy_page, true);
            cur_page_table[i] = next_phy_page;
            page_info_list[next_phy_page].SetInfo(arr_id, i);
        }
    }

    void MemoryManager::Release(ArrayList *arr) {
        auto &cur_page_table = page_table[arr->array_id];
        next_array_id ++;
        for (auto [i, phy_page]: cur_page_table) {
            freelist.set(phy_page, false);
            cur_page_table[i] = phy_page;
            page_info_list[phy_page].ClearInfo();
        }
    }

    int MemoryManager::find_page_to_evict() {
        // TODO:
        return 0;
    }

    std::string MemoryManager::build_page_file_name(int array_id, int vid) {
        // TODO:
        return "page";
    }

    FreeList::FreeList(size_t size) {
        // TODO:
    }

    bool FreeList::get(size_t idx) {
        // TODO:
    }

    void FreeList::set(size_t idx, bool val) {
        // TODO:
    }

    int FreeList::first_zero() const {
        // TODO:
    }

} // namespce: proj3