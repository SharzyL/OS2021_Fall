#ifndef THREAD_LIB_EMBEDDING_H_
#define THREAD_LIB_EMBEDDING_H_

#include <string>
#include <vector>
#include <memory>

namespace proj1 {

class Embedding{
public:
    explicit Embedding(int);  // Random init an embedding
    Embedding(int, double*, bool do_copy = true);
    Embedding(int, const std::string &);

    Embedding(const Embedding&);
    Embedding(Embedding&&) noexcept;
    Embedding& operator=(const Embedding &);
    Embedding& operator=(Embedding &&) noexcept;
    ~Embedding();

    [[nodiscard]] double* get_data() const;
    [[nodiscard]] int get_length() const;
    void update(const Embedding &, double);
    [[nodiscard]] std::string to_string() const;
    void write_to_stdout() const;

    // Operators
    Embedding operator+(const Embedding&) const;
    Embedding operator+(double) const;
    Embedding operator-(const Embedding&) const;
    Embedding operator-(double) const;
    Embedding operator*(const Embedding&) const;
    Embedding operator*(double) const;
    Embedding operator/(const Embedding&) const;
    Embedding operator/(double) const;
    bool operator==(const Embedding&) const;
    bool operator!=(const Embedding&) const;
private:
    int length = 0;
    double *data;
};

using EmbeddingMatrix = std::vector<Embedding>;
using EmbeddingGradient = Embedding;

class EmbeddingHolder{
public:
    EmbeddingHolder() = default;
    explicit EmbeddingHolder(const std::string& filename);
    static EmbeddingMatrix read(const std::string&);
    void write_to_stdout();
    void write(const std::string& filename);
    int append(const Embedding &embedding);
    void update_embedding(int, const EmbeddingGradient &, double);
    unsigned int get_n_embeddings();
    int get_emb_length();
    bool operator==(const EmbeddingHolder&);
    Embedding& operator[](int idx);
private:
    EmbeddingMatrix emb_matx;
};

} // namespace proj1
#endif // THREAD_LIB_EMBEDDING_H_
