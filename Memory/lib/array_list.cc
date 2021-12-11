#include "array_list.h"

#include "memory_manager.h"

namespace proj3 {
ArrayList::ArrayList(size_t sz, MemoryManager *cur_mma, int id) : array_id(id), mma(cur_mma), size(sz) {}

int ArrayList::Read(unsigned long idx) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    mma->ReadPage(array_id, (int)vid, (int)offset);
}

void ArrayList::Write(unsigned long idx, int value) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    mma->WritePage(array_id, (int)vid, (int)offset, value);
}
} // namespace proj3