#include <thread>
#include <vector>
#include <stdlib.h>

#include <benchmark/benchmark.h>

#include "lib/array_list.h"
#include "lib/memory_manager.h"

#include <glog/logging.h>

namespace proj3::testing {

class MMATest : public ::benchmark::Fixture {
protected:
    size_t workload_sz_1 = 4000;
    size_t workload_sz_2 = 2000;
    size_t workload_sz_3 = 100;
    size_t workload_sz_4 = 2000;
    int metrix_length = 10;
    int loop_times = 100;
    int thread_num = 10;

    int num_pages = 10;
    std::vector<std::vector<int>> metrix = {
        {2850, 2895, 2940, 2985, 3030, 3075, 3120, 3165, 3210, 3255},
        {7350, 7495, 7640, 7785, 7930, 8075, 8220, 8365, 8510, 8655},
        {11850, 12095, 12340, 12585, 12830, 13075, 13320, 13565, 13810, 14055},
        {16350, 16695, 17040, 17385, 17730, 18075, 18420, 18765, 19110, 19455},
        {20850, 21295, 21740, 22185, 22630, 23075, 23520, 23965, 24410, 24855},
        {25350, 25895, 26440, 26985, 27530, 28075, 28620, 29165, 29710, 30255},
        {29850, 30495, 31140, 31785, 32430, 33075, 33720, 34365, 35010, 35655},
        {34350, 35095, 35840, 36585, 37330, 38075, 38820, 39565, 40310, 41055},
        {38850, 39695, 40540, 41385, 42230, 43075, 43920, 44765, 45610, 46455},
        {43350, 44295, 45240, 46185, 47130, 48075, 49020, 49965, 50910, 51855},
    };

    void PostRun(benchmark::State &state, const proj3::MemoryManager &mma) {
        state.counters["miss"] = (double) mma.stat_num_miss;
        state.counters["access"] = (double) mma.stat_num_access;
        state.counters["miss_rate"] = (double) mma.stat_num_miss / (double) mma.stat_num_access;
    }
};

BENCHMARK_DEFINE_F(MMATest, task1_fifo)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_FIFO_ALG);
        proj3::ArrayList *arr = mma.Allocate((int) workload_sz_1);
        for (unsigned long i = 0; i < workload_sz_1; i++) {
            arr->Write(i, 1);
        }
        for (unsigned long i = 0; i < workload_sz_1; i++) {
            arr->Read(i);
        }
        mma.Release(arr);
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task1_clock)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_CLOCK_ALG);
        proj3::ArrayList *arr = mma.Allocate((int) workload_sz_1);
        for (unsigned long i = 0; i < workload_sz_1; i++) {
            arr->Write(i, 1);
        }
        for (unsigned long i = 0; i < workload_sz_1; i++) {
            arr->Read(i);
        }
        mma.Release(arr);
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task2_fifo)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_FIFO_ALG);
        std::vector<proj3::ArrayList *> arr;
        for (int i = 0; i < loop_times; i++) {
            arr.push_back(mma.Allocate(workload_sz_2));
            for (unsigned long j = 0; j < workload_sz_2; j++)
                arr[i]->Write(j, i);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2)
                mma.Release(arr[i]);
            else
                for (unsigned long j = 0; j < workload_sz_2; j++)
                    arr[i]->Read(j);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2 == 0)
                mma.Release(arr[i]);
        }
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task2_clock)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_CLOCK_ALG);
        std::vector<proj3::ArrayList *> arr;
        for (int i = 0; i < loop_times; i++) {
            arr.push_back(mma.Allocate(workload_sz_2));
            for (unsigned long j = 0; j < workload_sz_2; j++)
                arr[i]->Write(j, i);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2)
                mma.Release(arr[i]);
            else
                for (unsigned long j = 0; j < workload_sz_2; j++)
                    arr[i]->Read(j);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2 == 0)
                mma.Release(arr[i]);
        }
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task2_var_page_num)(benchmark::State &state) {
    for (auto _: state) {
        num_pages = (int) state.range(0);
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_CLOCK_ALG);
        std::vector<proj3::ArrayList *> arr;
        for (int i = 0; i < loop_times; i++) {
            arr.push_back(mma.Allocate(workload_sz_2));
            for (unsigned long j = 0; j < workload_sz_2; j++)
                arr[i]->Write(j, i);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2)
                mma.Release(arr[i]);
            else
                for (unsigned long j = 0; j < workload_sz_2; j++)
                    arr[i]->Read(j);
        }
        for (int i = 0; i < loop_times; i++) {
            if (i % 2 == 0)
                mma.Release(arr[i]);
        }
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task3_fifo)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_FIFO_ALG);
        std::vector<proj3::ArrayList *> metrixA, metrixB, metrixC;
        for (int i = 0; i < metrix_length; i++) {
            metrixA.push_back(mma.Allocate(metrix_length));
            metrixB.push_back(mma.Allocate(metrix_length));
            metrixC.push_back(mma.Allocate(metrix_length));
            for (int j = 0; j < metrix_length; j++) {
                metrixA[i]->Write(j, i * metrix_length + j);
                metrixB[i]->Write(j, i * metrix_length + j);
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            for (int j = 0; j < metrix_length; j++) {
                for (int k = 0; k < metrix_length; k++) {
                    metrixC[i]->Write(j, metrixC[i]->Read(j) + metrixA[i]->Read(k) * metrixB[k]->Read(j));
                }
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            for (int j = 0; j < metrix_length; j++) {
                metrixC[i]->Read(j);
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            mma.Release(metrixA[i]);
            mma.Release(metrixB[i]);
            mma.Release(metrixC[i]);
        }
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task3_clock)(benchmark::State &state) {
    for (auto _: state) {
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_CLOCK_ALG);
        std::vector<proj3::ArrayList *> metrixA, metrixB, metrixC;
        for (int i = 0; i < metrix_length; i++) {
            metrixA.push_back(mma.Allocate(metrix_length));
            metrixB.push_back(mma.Allocate(metrix_length));
            metrixC.push_back(mma.Allocate(metrix_length));
            for (int j = 0; j < metrix_length; j++) {
                metrixA[i]->Write(j, i * metrix_length + j);
                metrixB[i]->Write(j, i * metrix_length + j);
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            for (int j = 0; j < metrix_length; j++) {
                for (int k = 0; k < metrix_length; k++) {
                    metrixC[i]->Write(j, metrixC[i]->Read(j) + metrixA[i]->Read(k) * metrixB[k]->Read(j));
                }
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            for (int j = 0; j < metrix_length; j++) {
                metrixC[i]->Read(j);
            }
        }
        for (int i = 0; i < metrix_length; i++) {
            mma.Release(metrixA[i]);
            mma.Release(metrixB[i]);
            mma.Release(metrixC[i]);
        }
        PostRun(state, mma);
    }
}

BENCHMARK_DEFINE_F(MMATest, task4)(benchmark::State &state) {
    for (auto _: state) {
        thread_num = (int) state.range(0);
        proj3::MemoryManager mma(num_pages, MemoryManager::EVICT_CLOCK_ALG);
        std::vector<std::thread> pool;
        pool.reserve(thread_num);
        for (int i = 0; i < thread_num; i++) {
            pool.emplace_back([=, &mma] {
                proj3::ArrayList *arr = mma.Allocate((int) workload_sz_4);
                for (int j = 0; j < (int) workload_sz_4; j++)
                    arr->Write(j, j);
                for (int j = 0; j < (int) workload_sz_4; j++)
                    arr->Read(j);
                mma.Release(arr);
            });
        }
        for (auto &t : pool) {
            t.join();
        }
        PostRun(state, mma);
    }
}

// q1
BENCHMARK_REGISTER_F(MMATest, task1_fifo);
BENCHMARK_REGISTER_F(MMATest, task2_fifo);
BENCHMARK_REGISTER_F(MMATest, task3_fifo);

// q2
BENCHMARK_REGISTER_F(MMATest, task1_clock);
BENCHMARK_REGISTER_F(MMATest, task2_clock);
BENCHMARK_REGISTER_F(MMATest, task3_clock);

// q3
BENCHMARK_REGISTER_F(MMATest, task2_var_page_num)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6)
    ->Arg(7)
    ->Arg(8)
    ->Arg(9)
    ->Arg(10)
;

// q4
BENCHMARK_REGISTER_F(MMATest, task4)
    ->Arg(10)
    ->Arg(11)
    ->Arg(12)
    ->Arg(13)
    ->Arg(14)
    ->Arg(15)
    ->Arg(16)
    ->Arg(17)
    ->Arg(18)
    ->Arg(19)
    ->Arg(20)
;

} // namespace proj3

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
