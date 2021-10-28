#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include "lib/embedding.h"
#include "lib/instruction.h"
#include "lib/operation.h"

namespace proj1 {
class WorkerForBenchmark : public Worker {
public:
    WorkerForBenchmark(EmbeddingHolder &users, EmbeddingHolder &items, const Instructions &instructions)
        : Worker(users, items, instructions) {}

    void output_recommendation(const Embedding &recommendation) override {}

    EmbeddingHolder *recommendations;
};
} // namespace proj1

#define BQ(idx)                                                                                                        \
    static void BM_q##idx(benchmark::State &state) {                                                                   \
        for (auto _ : state) {                                                                                         \
            proj1::EmbeddingHolder users("data/q" #idx ".in");                                                         \
            proj1::EmbeddingHolder items("data/q" #idx ".in");                                                         \
            proj1::Instructions instructions = proj1::instr_from_file("data/q" #idx "_instruction.tsv");               \
            proj1::WorkerForBenchmark w(users, items, instructions);                                                   \
            w.work();                                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
    BENCHMARK(BM_q##idx);

BQ(1)
BQ(2)
BQ(3)
BQ(4)

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
