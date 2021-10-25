#include <gtest/gtest.h>
#include <vector>
#include <string>   // string

#include "model.h"
#include "embedding.h"
#include "instruction.h"
#include "operation.h"

#include "glog/logging.h"
#include "fmt/core.h"
#include "fmt/ranges.h"

namespace proj1 {

template <typename V>
bool vec_equal(V v1, V v2) {
    if (v1.size() != v2.size()) return false;
    for (int i = 0; i < v1.size(); i++) {
        if (v1[i] != v2[i]) return false;
    }
    return true;
}

class WorkerForTest : public Worker {
public:
    WorkerForTest(EmbeddingHolder &users, EmbeddingHolder &items, Instructions &instructions, std::vector<Embedding> *recommendations)
            : Worker(users, items, instructions), recommendations(recommendations)
    {
    }

    void outputRecommendation(Embedding *recommendation) {
        std::unique_lock<std::mutex> print_guard(printer_lock);
        recommendation->write_to_stdout();
    }
    std::vector<Embedding> *recommendations;
};

void run_one_instruction(Instruction inst, EmbeddingHolder *users, EmbeddingHolder *items, std::vector<Embedding>* recommendations=nullptr) {
    switch (inst.order) {
        case INIT_EMB: {
            // We need to init the embedding
            int length = users->get_emb_length();
            Embedding *new_user = new Embedding(length);
            int user_idx = users->append(new_user);
            for (int item_index: inst.payloads) {
                Embedding *item_emb = items->get_embedding(item_index);
                // Call cold start for downstream applications, slow
                EmbeddingGradient *gradient = cold_start(new_user, item_emb);
                users->update_embedding(user_idx, gradient, 0.01);
            }
            break;
        }
        case UPDATE_EMB: {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];
            // You might need to add this state in other questions.
            // Here we just show you this as an example
            // int epoch = -1;
            //if (inst.payloads.size() > 3) {
            //    epoch = inst.payloads[3];
            //}
            Embedding *user = users->get_embedding(user_idx);
            Embedding *item = items->get_embedding(item_idx);
            EmbeddingGradient *gradient = calc_gradient(user, item, label);
            users->update_embedding(user_idx, gradient, 0.01);
            gradient = calc_gradient(item, user, label);
            items->update_embedding(item_idx, gradient, 0.001);
            break;
        }
        case RECOMMEND: {
            int user_idx = inst.payloads[0];
            Embedding *user = users->get_embedding(user_idx);
            std::vector<Embedding *> item_pool;
            int iter_idx = inst.payloads[1];
            for (unsigned int i = 2; i < inst.payloads.size(); ++i) {
                int item_idx = inst.payloads[i];
                item_pool.push_back(items->get_embedding(item_idx));
            }
            Embedding *recommendation = recommend(user, item_pool);
            if (recommendations != nullptr) {
                recommendations->push_back(recommendation);
            } else {
                recommendation->write_to_stdout();
            }
            break;
        }
    }
}

class InitData {
public:
    InitData();
    EmbeddingHolder users1;
    EmbeddingHolder users2;
    EmbeddingHolder items1;
    EmbeddingHolder items2;
    EmbeddingHolder my_users;
    EmbeddingHolder my_items;
};

InitData::InitData():
        users1("data/q1.in"),
        users2("data/q1.in"),
        items1("data/q1.in"),
        items2("data/q1.in"),
        my_users("data/q1.in"),
        my_items("data/q1.in")
{
}

TEST(TestInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "0 0 1";
    std::string insr_str2 = "0 1";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestUpdate, ThreadSafe1) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 0";
    std::string insr_str2 = "1 0 2 1";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2);
    }
    
    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestUpdate, ThreadSafe2) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 1 0";
    std::string insr_str2 = "1 0 0 1 1";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestUpdate, EpochCorrectness) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "1 1 0 2 1";
    std::string insr_str3 = "1 -1 0 2 1";
    proj1::Instructions instructions1 = read_instructrions(insr_str3 + "\n" + insr_str1 + "\n" + insr_str2);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1
    );
}

TEST(TestRecommend, ThreadSafeAndCorrectness) {
    InitData init_data;
    std::string insr_str1 = "2 0 -1 1 2 3 4";
    std::string insr_str2 = "2 0 -1 3 4 5 6";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    std::vector<Embedding> recommend1;
    std::vector<Embedding> recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2, &recommend2);
    }
    
    std::vector<Embedding> my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            vec_equal(recommend1, my_recommend) || vec_equal(recommend2, my_recommend)
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestRecommendAndInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "2 0 -1 1 2 3 4";
    std::string insr_str2 = "0 0 1 2 3";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    std::vector<Embedding> recommend1;
    std::vector<Embedding> recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2, &recommend2);
    }
    
    std::vector<Embedding> my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();


    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            vec_equal(recommend1, my_recommend) || vec_equal(recommend2, my_recommend)
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestUpdateAndInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "0 0 1 2 3";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestUpdateAndRecommend, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 0";
    std::string insr_str2 = "2 0 0 0 1 2";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);
    std::vector<Embedding> recommend1;
    std::vector<Embedding> recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2, &recommend2);
    }
    
    std::vector<Embedding> my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();


    EXPECT_TRUE(
            init_data.my_users == init_data.users1 || init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            vec_equal(recommend1, my_recommend) || vec_equal(recommend2, my_recommend)
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items1 || init_data.my_items == init_data.items2
    );
}

TEST(TestRecommend, Epoch) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "2 0 -1 0 1 2";
    proj1::Instructions instructions1 = read_instructrions(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructrions(insr_str2 + "\n" + insr_str1);
    std::vector<Embedding> recommend1;
    std::vector<Embedding> recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, &init_data.users1, &init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, &init_data.users2, &init_data.items2, &recommend2);
    }
    
    std::vector<Embedding> my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();


    EXPECT_TRUE(
            init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            vec_equal(recommend2, my_recommend)
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items2
    );
}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace proj1
