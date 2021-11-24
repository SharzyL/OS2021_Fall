// process `INIT_EMB` and `UPDATE_EMB`, `RECOMMEND` instructions concurrently

#include <glog/logging.h>

#include "lib/embedding.h"
#include "lib/instruction.h"
#include "lib/operation.h"

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    proj1::EmbeddingHolder users("data/q4.in");
    proj1::EmbeddingHolder items("data/q4.in");
    proj1::Instructions instructions = proj1::instr_from_file("data/q4_instruction.tsv");

    proj1::Worker w(users, items, instructions);
    w.work();
}
