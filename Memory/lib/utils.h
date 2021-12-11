#ifndef DEADLOCK_LIB_UTILS_H_
#define DEADLOCK_LIB_UTILS_H_

#include <chrono>
#include <string>

namespace proj3 {

void a_slow_function(int seconds);

class AutoTimer {
public:
    AutoTimer(std::string name);
    ~AutoTimer();

private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
};

} // namespace proj3

#endif