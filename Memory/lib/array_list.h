#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <cstdlib>

#include "memory_manager.h"

namespace proj3 {

class MemoryManager; // placeholder for cyclic dependency

class ArrayList {
private:
    friend class MemoryManager;
    int size;
    MemoryManager *mma;
    int array_id;

public:
    // you should not modify the public interfaces used in tests
    ArrayList(int sz, MemoryManager *mma, int id);
    int Read(unsigned long);
    void Write(unsigned long, int);
};

} // namespace proj3
#endif