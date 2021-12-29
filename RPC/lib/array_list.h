#ifndef MEMORY_ARRAY_LIST_H
#define MEMORY_ARRAY_LIST_H

#include "mma_client.h"

namespace proj4 {

class MmaClient;

class ArrayList {
private:
    friend class MemoryManager;

    int size;
    MmaClient *mma;
    int array_id;

public:
    // you should not modify the public interfaces used in tests
    ArrayList(MmaClient *mma, int size, int id);

    int Read(unsigned long idx);

    void Write(unsigned long idx, int value);
};

} // namespace proj4
#endif // MEMORY_ARRAY_LIST_H
