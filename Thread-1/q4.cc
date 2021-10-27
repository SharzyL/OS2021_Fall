// process `INIT_EMB` and `UPDATE_EMB`, `RECOMMEND` instructions concurrently

#include "lib/embedding.h"
#include "lib/instruction.h"
#include "lib/operation.h"

int main() {
    auto users = proj1::EmbeddingHolder("data/q4.in");
    auto items = proj1::EmbeddingHolder("data/q4.in");
    proj1::Instructions instructions = proj1::instr_from_file("data/q4_instruction.tsv");

    proj1::Worker w(users, items, instructions);
    w.work();

    users.write_to_stdout();
    items.write_to_stdout();

    return 0;
}
