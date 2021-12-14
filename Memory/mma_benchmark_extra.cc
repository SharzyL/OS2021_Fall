//
// Created by sharzy on 12/13/21.
//

#include <fstream>
#include <random>
#include <thread>

#include <benchmark/benchmark.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#include "lib/array_list.h"
#include "lib/memory_manager.h"

int rand_int() { return std::rand(); }

class MMATest : public benchmark::Fixture {
protected:
    void Init(benchmark::State &state) {
        phy_pages = (int) state.range(0);
        app_num = (int) state.range(1);
        page_per_app = (int) state.range(2);
        iter_num = (int) state.range(3);

        mma = new proj3::MemoryManager(phy_pages);
        app_list.reserve(app_num);
        for (int i = 0; i < app_num; i++) {
            app_list.emplace_back(mma->Allocate(page_per_app * PageSize));
        }
    }

    void Clean() {
        delete mma;
        app_list.clear();
    }

    proj3::MemoryManager *mma;
    std::vector<proj3::ArrayList *> app_list;
    int phy_pages, app_num, page_per_app, iter_num;
};

BENCHMARK_DEFINE_F(MMATest, random_single_thread)(benchmark::State &state) {
    for (auto _ : state) {
        Init(state);
        for (int i = 0; i < iter_num * app_num; i++) {
            if (rand_int() % 2) {
                app_list[rand_int() % app_num]->Read(rand_int() % (PageSize * page_per_app));
            } else {
                app_list[rand_int() % app_num]->Write(rand_int() % (PageSize * page_per_app), 114514);
            }
        }
        Clean();
    }
}

BENCHMARK_DEFINE_F(MMATest, random_single_thread_in_order)(benchmark::State &state) {
    for (auto _ : state) {
        Init(state);
        for (int app = 0; app < app_num; app++) {
            for (int i = 0; i < iter_num; i++) {
                if (rand_int() % 2) {
                    app_list[app]->Read(rand_int() % (PageSize * page_per_app));
                } else {
                    app_list[app]->Write(rand_int() % (PageSize * page_per_app), 114514);
                }
            }
        }
        Clean();
    }
}

BENCHMARK_DEFINE_F(MMATest, random_multi_thread)(benchmark::State &state) {
    for (auto _ : state) {
        Init(state);

        std::vector<std::thread> jobs;
        jobs.reserve(app_num);
        for (int app = 0; app < app_num; app++) {
            jobs.emplace_back([=] {
                for (int i = 0; i < iter_num; i++) {
                    if (rand_int() % 2) {
                        app_list[app]->Read(rand_int() % (PageSize * page_per_app));
                    } else {
                        app_list[app]->Write(rand_int() % (PageSize * page_per_app), 114514);
                    }
                }
            });
        }
        for (auto &job: jobs) {
            job.join();
        }
        Clean();
    }
}

BENCHMARK_REGISTER_F(MMATest, random_single_thread)
    ->Args({1000, 40, 30, 1000})
    ->Args({1000, 40, 60, 1000});
BENCHMARK_REGISTER_F(MMATest, random_single_thread_in_order)
    ->Args({1000, 40, 30, 1000})
    ->Args({1000, 40, 60, 1000});
BENCHMARK_REGISTER_F(MMATest, random_multi_thread)->UseRealTime()
    ->Args({1000, 40, 30, 1000})
    ->Args({1000, 40, 60, 1000});

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
