#include "utils.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace proj2 {

void a_slow_function(int seconds) { std::this_thread::sleep_for(std::chrono::seconds(seconds) / 1000.0); }

int randint(int lower, int upper) { return rand() % (upper - lower + 1) + lower; }

bool randbit() { return rand() % 2 > 0; }

AutoTimer::AutoTimer(std::string name) : m_name(std::move(name)), m_beg(std::chrono::high_resolution_clock::now()) {}

AutoTimer::~AutoTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - m_beg);
    std::cout << m_name << " : " << dur.count() << " usec\n";
}

} // namespace proj2
