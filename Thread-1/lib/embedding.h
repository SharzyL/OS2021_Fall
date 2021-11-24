#ifndef THREAD_LIB_EMBEDDING_H_
#define THREAD_LIB_EMBEDDING_H_

#include <memory>
#include <string>
#include <vector>

namespace proj1 {

class Embedding {
public:
    Embedding() = default;
    explicit Embedding(int); // Random init an embedding
    Embedding(int, double *, bool do_copy = true);
    Embedding(int, const std::string &);

    Embedding(const Embedding &);
    Embedding(Embedding &&) noexcept;
    Embedding &operator=(const Embedding &);
    Embedding &operator=(Embedding &&) noexcept;
    ~Embedding();

    [[nodiscard]] double *get_data() const;
    [[nodiscard]] int get_length() const;
    void update(const Embedding &, double);

    friend std::ostream &operator<<(std::ostream &output, const Embedding &holder);
    void write_to_stdout() const;

    // Operators
    Embedding operator+(const Embedding &) const;
    Embedding operator+(double) const;
    Embedding operator-(const Embedding &) const;
    Embedding operator-(double) const;
    Embedding operator*(const Embedding &) const;
    Embedding operator*(double) const;
    Embedding operator/(const Embedding &) const;
    Embedding operator/(double) const;
    bool operator==(const Embedding &) const;
    bool operator!=(const Embedding &) const;

private:
    int length = 0;
    double *data = nullptr;
};

using EmbeddingGradient = Embedding;

class EmbeddingHolder {
public:
    EmbeddingHolder() = default;
    explicit EmbeddingHolder(const std::string &filename);
    explicit EmbeddingHolder(std::vector<Embedding> emb_vector) : emb_matx(std::move(emb_vector)){};

    friend std::ostream &operator<<(std::ostream &output, const EmbeddingHolder &holder);
    void write_to_stdout() const;
    void write(const std::string &filename) const;

    template <typename... T> int emplace_back(T &&...args) { // universal reference
        emb_matx.emplace_back(std::forward<T>(args)...);
        return (int)emb_matx.size() - 1;
    }

    void update_embedding(int, const EmbeddingGradient &, double);
    [[nodiscard]] int get_n_embeddings() const;
    [[nodiscard]] int get_emb_length() const;
    bool operator==(const EmbeddingHolder &) const;
    Embedding &operator[](int idx);

private:
    std::vector<Embedding> emb_matx;
};

} // namespace proj1
#endif // THREAD_LIB_EMBEDDING_H_
