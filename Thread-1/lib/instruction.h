#ifndef THREAD_LIB_INSTRUCTION_H_
#define THREAD_LIB_INSTRUCTION_H_

#include <string>
#include <vector>

namespace proj1 {

enum InstructionOrder {
    INIT_EMB = 0,
    UPDATE_EMB,
    RECOMMEND
};

struct Instruction {
    explicit Instruction(std::string);
    InstructionOrder order;
    std::vector<int> payloads;
};

using Instructions = std::vector<Instruction>;

Instructions read_instructions(const std::string &filename);
Instructions read_instructions_from_str(const std::string &str);

} // namespace proj1
#endif  // THREAD_LIB_INSTRUCTION_H_