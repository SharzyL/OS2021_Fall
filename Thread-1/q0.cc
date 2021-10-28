#include "lib/embedding.h"
#include "lib/instruction.h"
#include "lib/operation.h"

int main(int argc, char *argv[]) {
    proj1::EmbeddingHolder users("data/q0.in");
    proj1::EmbeddingHolder items("data/q0.in");
    proj1::Instructions instructions = proj1::instr_from_file("data/q0_instruction.tsv");

    // Run all the instructions
    for (const auto &inst: instructions) {
        proj1::run_one_instruction(inst, users, items);
    }

    // Write the result
    users.write_to_stdout();
    items.write_to_stdout();
}
