//
// Created by sharzy on 12/13/21.
//

#include <fstream>
#include <random>
#include <thread>

#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include "lib/array_list.h"
#include "lib/memory_manager.h"

int rand_int() { return std::rand(); }

static void BM_random(benchmark::State &state) {
    for (auto _ : state) {
        state.PauseTiming();
        const int phy_pages = 1000;
        const int app_num = 100;
        const int page_per_app = 50;
        const int iter_num = 100000;

        auto mma = new proj3::MemoryManager(phy_pages);
        std::vector<proj3::ArrayList *> appList;
        appList.reserve(app_num);
        for (int i = 0; i < 100; i++) {
            appList.emplace_back(mma->Allocate(page_per_app * PageSize));
        }
        state.ResumeTiming();
        for (int i = 0; i < iter_num; i++) {
            if (rand_int() % 2) {
                appList[rand_int() % app_num]->Read(rand_int() % (PageSize * page_per_app));
            } else {
                appList[rand_int() % app_num]->Write(rand_int() % (PageSize * page_per_app), 114514);
            }
        }
    }
}

static void BM_disk_concur(benchmark::State &state) {
    for (auto _ : state) {
        state.PauseTiming();
        size_t num_files = state.range(0);
        size_t num_bytes = state.range(1);

        char *src = new char[num_bytes];
        for (size_t i = 0; i < num_bytes; i++) {
            src[i] = (char)(1 + rand_int() % 255);
        }
        std::vector<std::thread> workers;
        workers.reserve(num_files);
        state.ResumeTiming();
        for (size_t i = 0; i < num_files; i++) {
            workers.emplace_back([=] {
                std::string filename = "file_" + std::to_string(i);
                std::ofstream of(filename);
                of.write(src, (long)num_bytes);
                std::flush(of);
            });
        }
        for (auto &th : workers) {
            th.join();
        }
    }
}

static void BM_disk(benchmark::State &state) {
    for (auto _ : state) {
        state.PauseTiming();
        size_t num_files = state.range(0);
        size_t num_bytes = state.range(1);
        char *src = new char[num_bytes];
        for (size_t i = 0; i < num_bytes; i++) {
            src[i] = (char)(1 + rand_int() % 255);
        }
        state.ResumeTiming();
        for (size_t i = 0; i < num_files; i++) {
            std::string filename = "file_" + std::to_string(i);
            std::ofstream of(filename);
            of.write(src, (long)num_bytes);
            std::flush(of);
        }
        delete[] src;
    }
}

BENCHMARK(BM_random);

BENCHMARK(BM_disk_concur)->Args({100, 1 << 12})->Args({100, 1 << 16});
BENCHMARK(BM_disk)->Args({100, 1 << 12})->Args({100, 1 << 16});

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
