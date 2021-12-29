#include "array_list.h"
#include "const.h"

namespace proj4 {

int proj4::ArrayList::Read(unsigned long idx) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    return mma->ReadPage(array_id, (int)vid, (int)offset);
}

void proj4::ArrayList::Write(unsigned long idx, int value) {
    size_t vid = idx / PageSize;
    size_t offset = idx % PageSize;
    mma->WritePage(array_id, (int)vid, (int)offset, value);
}

proj4::ArrayList::ArrayList(proj4::MmaClient *mma, int size, int id) : size(size), mma(mma), array_id(id) {}

} // namespace proj4