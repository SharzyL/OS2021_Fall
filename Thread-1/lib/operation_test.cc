#include <gtest/gtest.h>
#include <string> // string
#include <vector>

#include "lib/embedding.h"
#include "lib/instruction.h"
#include "lib/operation.h"

#include "glog/logging.h"

namespace proj1 {
namespace testing {

const std::string default_data_file = "data/q1.in";

class WorkerForTest : public Worker {
public:
    WorkerForTest(EmbeddingHolder &users, EmbeddingHolder &items, const Instructions &instructions,
                  EmbeddingHolder *recommendations)
        : Worker(users, items, instructions), recommendations(recommendations) {}

    void output_recommendation(const Embedding &recommendation) override {
        std::unique_lock<std::mutex> print_guard(printer_lock);
        recommendations->emplace_back(std::move(recommendation));
    }

    EmbeddingHolder *recommendations;
};

bool testWorker(const std::string &data_file, const std::vector<Instructions> &instr_refs,
                const Instructions &instr_test) {
    EmbeddingHolder recom_test;
    EmbeddingHolder users_test(data_file), items_test(data_file);
    WorkerForTest worker(users_test, items_test, instr_test, &recom_test);
    worker.work();

    for (const auto &instr_ref : instr_refs) {
        EmbeddingHolder recom_ref;
        EmbeddingHolder users_ref(data_file), items_ref(data_file);
        for (const auto &instr : instr_ref) {
            run_one_instruction(instr, users_ref, items_ref, &recom_ref);
        }
        if (recom_test == recom_ref && users_test == users_ref && items_test == items_ref)
            return true;
    }
    return false;
}

bool testWorker(const std::string &data_file, const Instructions &instr_test) {
    return testWorker(data_file, std::vector<Instructions>{instr_test}, instr_test);
}

TEST(TestInit, ThreadSafe) {
    std::string instr1 = "0 0 1";
    std::string instr2 = "0 1";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestUpdate, Once) {
    std::string instr1 = "1 0 1 0";
    Instructions instr_seq = instr_from_str(instr1);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

TEST(TestUpdate, ThreadSafe1) {
    std::string instr1 = "1 0 1 0";
    std::string instr2 = "1 0 2 1";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestUpdate, ThreadSafe2) {
    std::string instr1 = "1 0 1 1 0";
    std::string instr2 = "1 0 0 1 1";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestUpdate, EpochCorrectness) {
    std::string instr1 = "1 1 1 1 1";
    std::string instr2 = "1 2 1 2 2";
    std::string instr3 = "1 1 0 2 0";
    Instructions instr_seq = instr_from_str(instr3 + "\n" + instr1 + "\n" + instr2);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

TEST(TestRecommend, ThreadSafeAndCorrectness) {
    std::string instr1 = "2 0 -1 1 2 3 4";
    std::string instr2 = "2 0 -1 3 4 5 6";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestRecommendAndInit, ThreadSafe) {
    std::string instr1 = "2 0 -1 1 2 3 4";
    std::string instr2 = "0 0 1 2 3";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestUpdateAndInit, ThreadSafe) {
    std::string instr1 = "1 0 0 1 0";
    std::string instr2 = "0 0 1 2 3";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestUpdateAndRecommend, ThreadSafe) {
    std::string instr1 = "1 0 1 0";
    std::string instr2 = "2 0 0 0 1 2";
    Instructions instr_seq1 = instr_from_str(instr1 + "\n" + instr2);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestRecommend, Epoch1) {
    std::string instr1 = "1 0 0 1 1";
    std::string instr2 = "2 0 -1 0 1";
    std::string instr3 = "2 0 0 0 1";
    Instructions instr_seq = instr_from_str(instr2 + "\n" + instr3 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

TEST(TestRecommend, Epoch2) {
    std::string instr1 = "1 0 0 0 0";
    std::string instr2 = "2 0 1 0 1";
    std::string instr3 = "2 0 2 0 1";
    Instructions instr_seq = instr_from_str(instr2 + "\n" + instr3 + "\n" + instr1);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

TEST(TestRecommendAndUpdate, Epoch1) {
    std::string instr1 = "1 0 0 1 0";
    std::string instr2 = "1 0 0 0 0";
    std::string instr3 = "2 0 -1 0 1";
    std::string instr4 = "2 1 2 0 1";
    Instructions instr_seq1 = instr_from_str(instr3 + "\n" + instr2 + "\n" + instr1 + "\n" + instr4);
    Instructions instr_seq2 = instr_from_str(instr2 + "\n" + instr1 + "\n" + instr2 + "\n" + instr4);
    EXPECT_TRUE(testWorker(default_data_file, std::vector{instr_seq1, instr_seq2}, instr_seq1));
}

TEST(TestRecommendAndUpdate, Epoch2) {
    std::string instr1 = "1 0 0 1 0";
    std::string instr2 = "1 0 0 0 1";
    std::string instr3 = "2 0 -1 0 1";
    std::string instr4 = "2 1 2 0 1";
    Instructions instr_seq = instr_from_str(instr3 + "\n" + instr1 + "\n" + instr2 + "\n" + instr4);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

TEST(TestWorkload, Epoch) {
    std::string instr1 = "2 0 -1 0 1"; // epoch -1
    std::string instr2 = "1 0 0 0 0";  // epoch 0
    std::string instr3 = "1 0 0 1 1";  // epoch 1
    std::string instr4 = "2 1 2 0 1";  // epoch 2
    std::string instr_seq1 = "";
    std::string instr_seq2 = "";
    std::string instr_seq3 = "";
    std::string instr_seq4 = "";
    for (int i = 0; i < 5; i++) {
        std::string trailing_char = i < 4 ? "\n" : "";
        instr_seq1 += instr1 + trailing_char;
        instr_seq2 += instr2 + trailing_char;
        instr_seq3 += instr3 + trailing_char;
        instr_seq4 += instr4 + trailing_char;
    }
    Instructions instr_seq = instr_from_str(instr_seq1 + "\n" + instr_seq2 + "\n" + instr_seq3 + "\n" + instr_seq4);
    EXPECT_TRUE(testWorker(default_data_file, instr_seq));
}

} // namespace testing
} // namespace proj1

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}