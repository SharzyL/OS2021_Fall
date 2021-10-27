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

class WorkerForTest : public Worker {
public:
    WorkerForTest(EmbeddingHolder &users, EmbeddingHolder &items, Instructions &instructions, EmbeddingHolder *recommendations)
            : Worker(users, items, instructions), recommendations(recommendations)
    {
    }

    void output_recommendation(const Embedding &recommendation) override {
        std::unique_lock<std::mutex> print_guard(printer_lock);
        recommendations->append(recommendation);
    }
    EmbeddingHolder *recommendations;
};

void run_one_instruction(Instruction inst, EmbeddingHolder &users, EmbeddingHolder &items, EmbeddingHolder* recommendations=nullptr) {
    switch (inst.order) {
        case INIT_EMB: {
            // We need to init the embedding
            int length = users.get_emb_length();
            Embedding new_user(length);
            LOG(ERROR) << new_user.to_string();
            int user_idx = users.append(new_user);
            for (int item_index: inst.payloads) {
                Embedding item_emb = items.get_embedding(item_index);
                // Call cold start for downstream applications, slow
                EmbeddingGradient gradient = cold_start(new_user, item_emb);
                users.update_embedding(user_idx, gradient, 0.01);
            }
            break;
        }
        case UPDATE_EMB: {
            int user_idx = inst.payloads[0];
            int item_idx = inst.payloads[1];
            int label = inst.payloads[2];
            Embedding user = users.get_embedding(user_idx);
            Embedding item = items.get_embedding(item_idx);
            EmbeddingGradient user_gradient = calc_gradient(user, item, label);
            users.update_embedding(user_idx, user_gradient, 0.01);
            EmbeddingGradient item_gradient = calc_gradient(item, user, label);
            items.update_embedding(item_idx, item_gradient, 0.001);
            break;
        }
        case RECOMMEND: {
            int user_idx = inst.payloads[0];
            Embedding user = users.get_embedding(user_idx);
            std::vector<Embedding> item_pool;
            for (unsigned int i = 2; i < inst.payloads.size(); ++i) {
                int item_idx = inst.payloads[i];
                item_pool.push_back(items.get_embedding(item_idx));
            }
            Embedding recommendation = recommend(user, item_pool);
            if (recommendations != nullptr) {
                recommendations->append(recommendation);
            } else {
                recommendation.write_to_stdout();
            }
            break;
        }
    }
}

class InitData {
public:
    InitData():
            users1("data/q1.in"),
            users2("data/q1.in"),
            items1("data/q1.in"),
            items2("data/q1.in"),
            my_users("data/q1.in"),
            my_items("data/q1.in")
    {}

    EmbeddingHolder users1;
    EmbeddingHolder users2;
    EmbeddingHolder items1;
    EmbeddingHolder items2;
    EmbeddingHolder my_users;
    EmbeddingHolder my_items;
};

TEST(TestInit, Once) {
    InitData init_data;
    std::string insr_str1 = "0 0 1";
    proj1::Instructions instructions = read_instructions_from_str(insr_str1);

//    for (proj1::Instruction inst: instructions) {
//        proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
//    }
//
    proj1::Worker w(init_data.my_users, init_data.my_items, instructions);
    w.work();

//    EXPECT_TRUE(
//            init_data.my_users == init_data.users1 && init_data.my_items == init_data.items1
//    );
}

TEST(TestInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "0 0 1";
    std::string insr_str2 = "0 1";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users1 && init_data.my_items == init_data.items1)
            || (init_data.my_users == init_data.users2 && init_data.my_items == init_data.items2)
    );
}

TEST(TestUpdate, ThreadSafe1) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 0";
    std::string insr_str2 = "1 0 2 1";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2);
    }
    
    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users1 && init_data.my_items == init_data.items1)
            || (init_data.my_users == init_data.users2 && init_data.my_items == init_data.items2)
    );
}

TEST(TestUpdate, ThreadSafe2) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 1 0";
    std::string insr_str2 = "1 0 0 1 1";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users1 && init_data.my_items == init_data.items1)
            || (init_data.my_users == init_data.users2 && init_data.my_items == init_data.items2)
    );
}

TEST(TestUpdate, EpochCorrectness) {
    for (int i = 0; i < 10; i++) {
        InitData init_data;
        std::string insr_str1 = "1 1 1 1 1";
        std::string insr_str2 = "1 2 1 2 2";
        std::string insr_str3 = "1 1 0 2 0";
        proj1::Instructions instructions1 = read_instructions_from_str(insr_str3 + "\n" + insr_str1 + "\n" + insr_str2);

        for (proj1::Instruction inst: instructions1) {
            proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
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
}

TEST(TestRecommend, ThreadSafeAndCorrectness) {
    InitData init_data;
    std::string insr_str1 = "2 0 -1 1 2 3 4";
    std::string insr_str2 = "2 0 -1 3 4 5 6";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    EmbeddingHolder recommend1;
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users2 && recommend2 == my_recommend && init_data.my_items == init_data.items2) || (init_data.my_users == init_data.users1 && recommend1 == my_recommend && init_data.my_items == init_data.items1)
    );
}

TEST(TestRecommendAndInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "2 0 -1 1 2 3 4";
    std::string insr_str2 = "0 0 1 2 3";
    Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    EmbeddingHolder recommend1;
    EmbeddingHolder recommend2;
    for (const auto &inst: instructions1) {
        run_one_instruction(inst, init_data.users1, init_data.items1, &recommend1);
    }
    for (const auto &inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users2 && recommend2 == my_recommend && init_data.my_items == init_data.items2) || (init_data.my_users == init_data.users1 && recommend1 == my_recommend && init_data.my_items == init_data.items1)
    );
}

TEST(TestUpdateAndInit, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "0 0 1 2 3";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);

    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2);
    }

    proj1::Worker w(init_data.my_users, init_data.my_items, instructions1);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users1 && init_data.my_items == init_data.items1) || (init_data.my_users == init_data.users2 && init_data.my_items == init_data.items2)
    );
}

TEST(TestUpdateAndRecommend, ThreadSafe) {
    InitData init_data;
    std::string insr_str1 = "1 0 1 0";
    std::string insr_str2 = "2 0 0 0 1 2";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str1 + "\n" + insr_str2);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1);
    EmbeddingHolder recommend1;
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users2 && recommend2 == my_recommend && init_data.my_items == init_data.items2) || (init_data.my_users == init_data.users1 && recommend1 == my_recommend && init_data.my_items == init_data.items1)
    );
}

TEST(TestRecommend, Epoch1) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 1";
    std::string insr_str2 = "2 0 -1 0 1";
    std::string insr_str3 = "2 0 0 0 1";
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str3 + "\n" + insr_str1);
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions2, &my_recommend);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            recommend2 == my_recommend
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items2
    );
}

TEST(TestRecommend, Epoch2) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 0 0";
    std::string insr_str2 = "2 0 1 0 1";
    std::string insr_str3 = "2 0 2 0 1";
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str1 + "\n" + insr_str2 + "\n" + insr_str3);
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions2, &my_recommend);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            recommend2 == my_recommend
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items2
    );
}

TEST(TestRecommendAndUpdate, Epoch1) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "1 0 0 0 0";
    std::string insr_str3 = "2 0 -1 0 1";
    std::string insr_str4 = "2 1 2 0 1";
    proj1::Instructions instructions1 = read_instructions_from_str(insr_str3 + "\n" + insr_str2+ "\n" + insr_str1 + "\n" + insr_str4);
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str2 + "\n" + insr_str1 + "\n" + insr_str2 + "\n" + insr_str4);
    EmbeddingHolder recommend1;
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions1) {
        proj1::run_one_instruction(inst, init_data.users1, init_data.items1, &recommend1);
    }
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
    w.work();

    EXPECT_TRUE(
            (init_data.my_users == init_data.users2 && recommend2 == my_recommend && init_data.my_items == init_data.items2) || (init_data.my_users == init_data.users1 && recommend1 == my_recommend && init_data.my_items == init_data.items1)
    );
}

TEST(TestRecommendAndUpdate, Epoch2) {
    InitData init_data;
    std::string insr_str1 = "1 0 0 1 0";
    std::string insr_str2 = "1 0 0 0 1";
    std::string insr_str3 = "2 0 -1 0 1";
    std::string insr_str4 = "2 1 2 0 1";
    proj1::Instructions instructions2 = read_instructions_from_str(insr_str3 + "\n" + insr_str1 + "\n" + insr_str2 + "\n" + insr_str4);
    EmbeddingHolder recommend2;
    for (proj1::Instruction inst: instructions2) {
        proj1::run_one_instruction(inst, init_data.users2, init_data.items2, &recommend2);
    }
    
    EmbeddingHolder my_recommend;
    proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions2, &my_recommend);
    w.work();

    EXPECT_TRUE(
            init_data.my_users == init_data.users2
    );
    EXPECT_TRUE(
            recommend2 == my_recommend
    );
    EXPECT_TRUE(
            init_data.my_items == init_data.items2
    );
}

TEST(TestWorkload, Epoch) {
        InitData init_data;
        std::string insr_str1 = "2 0 -1 0 1";  // epoch -1
        std::string insr_str2 = "1 0 0 0 0";   // epoch 0
        std::string insr_str3 = "1 0 0 1 1";   // epoch 1
        std::string insr_str4 = "2 1 2 0 1";   // epoch 2
        std::string insr1 = "";
        std::string insr2 = "";
        std::string insr3 = "";
        std::string insr4 = "";
        for(int i = 0; i < 100; i++) {
            std::string trailing_char = i < 99 ? "\n" : "";
            insr1 += insr_str1 + trailing_char;
            insr2 += insr_str2 + trailing_char;
            insr3 += insr_str3 + trailing_char;
            insr4 += insr_str4 + trailing_char;
        }
        proj1::Instructions instructions1 = read_instructions_from_str(insr1 + "\n" + insr2+ "\n" + insr3 + "\n" + insr4);
        EmbeddingHolder recommend1;
        for (proj1::Instruction inst: instructions1) {
            proj1::run_one_instruction(inst, init_data.users1, init_data.items1, &recommend1);
        }

        EmbeddingHolder my_recommend;
        proj1::WorkerForTest w(init_data.my_users, init_data.my_items, instructions1, &my_recommend);
        w.work();

        EXPECT_TRUE((
                init_data.my_users == init_data.users1
                && init_data.my_items == init_data.items1
                && recommend1 == my_recommend
        ));
    }

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace proj1
