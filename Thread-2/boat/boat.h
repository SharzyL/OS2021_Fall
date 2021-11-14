#ifndef BOAT_H_
#define BOAT_H_

#include <mutex>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#include "boatGrader.h"

namespace proj2 {
class Boat {
public:
    Boat();
    ~Boat(){};
    void begin(int, int, BoatGrader *);
};
} // namespace proj2

#endif // BOAT_H_
