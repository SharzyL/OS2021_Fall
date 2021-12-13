#include "array_list.h"

#include "memory_manager.h"

namespace proj3 {
ArrayList::ArrayList(int sz, MemoryManager *cur_mma, int id) : size(sz), mma(cur_mma), array_id(id) {}

int ArrayList::Read(unsigned long idx) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    return mma->ReadPage(array_id, (int)vid, (int)offset);
}

void ArrayList::Write(unsigned long idx, int value) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    mma->WritePage(array_id, (int)vid, (int)offset, value);
}
} // namespace proj3