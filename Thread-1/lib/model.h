#ifndef THREAD_LIB_MODEL_H_
#define THREAD_LIB_MODEL_H_

#include <vector>
#include "embedding.h"

namespace proj1 {

/* NOTE: DO NOT rely on the implementation here. We may
         change the implemenation details.
*/
double similarity(const Embedding &entityA, const Embedding &entityB);

EmbeddingGradient calc_gradient(const Embedding &entityA, const Embedding &entityB, int label);

EmbeddingGradient cold_start(const Embedding &newUser, const Embedding &item);

const Embedding& recommend(const Embedding &user, const std::vector<Embedding>& items);

} // namespace proj1

#endif // THREAD_LIB_MODEL_H_