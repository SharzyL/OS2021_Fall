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

Instructions instr_from_file(const std::string &filename);
Instructions instr_from_str(const std::string &str);

} // namespace proj1
#endif  // THREAD_LIB_INSTRUCTION_H_