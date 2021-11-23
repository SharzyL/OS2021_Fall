#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "boat.h"

namespace proj2 {

using namespace std::chrono_literals;

class Environment {
public:
    explicit Environment(BoatGrader &grader): grader(grader) {};

    void child_run() {
        auto guard = std::unique_lock<std::mutex>(boat_mutex);
        bool self_not_landed = true;
        bool self_in_oahu = true;

        while (true) {
            if (self_in_oahu) {
                // grab the boat to monokai
                owc ++;
                if (self_not_landed) {
                    guard.unlock();
                    std::this_thread::sleep_for(100ms);
                    guard.lock();
                    self_not_landed = false;
                }

                boat_on_oahu.wait(guard, [&, this] {
                    bool boat_not_full = ac <= 1 && aa == 0;
                    bool should_send_child = mwc == 0 || owa == 0;
                    return is_boat_on_oahu && boat_not_full && should_send_child;
                });
                owc --;
                ac ++;

                // decide who is the pilot
                bool is_pilot = (ac == 1);
                if (ac == 1) {
                    boat_full.wait(guard, [&, this] {
                        return ac == 2 || owc == 0;
                    });
                } else {
                    boat_full.notify_one();
                }

                // go to monokai
                if (is_pilot) {
                    grader.ChildRowToMolokai();
                    if (ac == 2) grader.ChildRideToMolokai();
                    is_boat_on_oahu = false;
                    ac = 0;
                    boat_on_monokai.notify_all();
                }
                self_in_oahu = false;
            } else {
                // grab the boat to oahu
                mwc ++;
                boat_on_monokai.wait(guard, [&, this] {
                    return !is_boat_on_oahu && ac + aa == 0;
                });
                if (owc + owa == 0) break;
                mwc --;
                ac ++;

                // go to oahu
                grader.ChildRowToOahu();
                is_boat_on_oahu = true;
                self_in_oahu = true;
                ac = 0;
                boat_on_oahu.notify_all();
            }
        }
    }

    void adult_run() {
        auto guard = std::unique_lock<std::mutex>(boat_mutex);
        // grab the boat to monokai
        owa++;
        guard.unlock();
        std::this_thread::sleep_for(100ms);
        guard.lock();

        boat_on_oahu.wait(guard, [&, this] {
            bool boat_not_full = ac + aa == 0;
            bool should_send_adult = mwc > 0;
            return is_boat_on_oahu && boat_not_full && should_send_adult;
        });
        aa++;
        owa--;

        grader.AdultRowToMolokai();
        is_boat_on_oahu = false;
        aa = 0;
        boat_on_monokai.notify_all();
    }

private:
    BoatGrader &grader;

    int owc = 0;  // oahu waiting children
    int owa = 0;  // oahu waiting adult
    int ac = 0;   // active children (ready to embark)
    int aa = 0;   // active parents
    int mwc = 0;  // monokai waiting children

    bool is_boat_on_oahu = true;

    std::condition_variable boat_on_oahu;
    std::condition_variable boat_on_monokai;
    std::condition_variable boat_full;

    std::mutex boat_mutex;
};

void Boat::begin(int a, int b, BoatGrader *bg) {
    std::vector<std::thread> jobs;
    Environment e(*bg);
    jobs.reserve(a + b);
    for (int i = 0; i < a; i++) {
        jobs.emplace_back(&Environment::adult_run, &e);
    }
    for (int i = 0; i < b; i++) {
        jobs.emplace_back(&Environment::child_run, &e);
    }
    for (auto &job : jobs) {
        job.join();
    }
}

} // namespace proj2
