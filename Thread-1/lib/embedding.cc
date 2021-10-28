#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "embedding.h"

namespace proj1 {

Embedding::Embedding(int length) {
    this->data = new double[length];
    for (int i = 0; i < length; ++i) {
        this->data[i] = (double)i / 10.0;
    }
    this->length = length;
}

Embedding::Embedding(int length, double *data, bool do_copy) {
    this->length = length;
    if (do_copy) {
        this->data = new double[length];
        std::memcpy(this->data, data, length * sizeof(double));
    } else {
        this->data = data;
    }
}

// Rule of five
Embedding::Embedding(const Embedding &other) : Embedding(other.length, other.data) {}

Embedding::Embedding(Embedding &&other) noexcept : length(other.length), data(std::exchange(other.data, nullptr)) {}

Embedding &Embedding::operator=(const Embedding &other) { return *this = Embedding(other); }

Embedding &Embedding::operator=(Embedding &&other) noexcept {
    length = other.length;
    std::swap(data, other.data);
    return *this;
}

Embedding::~Embedding() { delete[] data; }
// end rule of five

Embedding::Embedding(int length, const std::string &raw) {
    this->length = length;
    this->data = new double[length];
    std::stringstream ss(raw);
    for (int i = 0; (i < length) && (ss >> this->data[i]); ++i) {
        if (ss.peek() == ',')
            ss.ignore(); // Ignore the delimiter
    }
}

void Embedding::update(const Embedding &gradient, double stepsize) {
    for (int i = 0; i < this->length; ++i) {
        this->data[i] -= stepsize * gradient.data[i];
    }
}

std::ostream &operator<<(std::ostream &output, const Embedding &emb) {
    for (int i = 0; i < emb.length; ++i) {
        if (i > 0)
            output << ",";
        output << emb.data[i];
    }
    return output;
};

void Embedding::write_to_stdout() const {
    std::string prefix("[OUTPUT]");
    std::cout << prefix << *this << '\n';
}

Embedding Embedding::operator+(const Embedding &another) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] + another.data[i];
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator+(const double value) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] + value;
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator-(const Embedding &another) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] - another.data[i];
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator-(const double value) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] - value;
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator*(const Embedding &another) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] * another.data[i];
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator*(const double value) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] * value;
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator/(const Embedding &another) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] / another.data[i];
    }
    return {this->length, new_data, false};
}

Embedding Embedding::operator/(const double value) const {
    auto *new_data = new double[this->length];
    for (int i = 0; i < this->length; ++i) {
        new_data[i] = this->data[i] / value;
    }
    return {this->length, new_data, false};
}

bool Embedding::operator==(const Embedding &other) const {
    for (int i = 0; i < this->length; ++i) {
        if (std::abs(this->data[i] - other.data[i]) > 1.0e-6)
            return false;
    }
    return true;
}

bool Embedding::operator!=(const Embedding &other) const { return !this->operator==(other); }

double *Embedding::get_data() const { return this->data; }

int Embedding::get_length() const { return (int)this->length; }

EmbeddingHolder::EmbeddingHolder(const std::string &filename) {
    std::ifstream ifs(filename);
    int length = 0;
    std::vector<Embedding> matrix;

    std::string line;
    if (ifs.is_open()) {
        while (std::getline(ifs, line)) {
            if (length == 0) {
                for (char x : line) {
                    if (x == ',' || x == ' ')
                        ++length;
                }
                ++length;
            }
            matrix.emplace_back(length, line);
        }
        ifs.close();
    } else {
        throw std::runtime_error("Error opening file " + filename);
    }
    this->emb_matx = std::move(matrix);
}

std::ostream &operator<<(std::ostream &output, const EmbeddingHolder &holder) {
    for (const auto &emb : holder.emb_matx) {
        output << emb << '\n';
    }
    return output;
}

void EmbeddingHolder::write(const std::string &filename) const {
    std::ofstream ofs(filename);
    if (ofs.is_open()) {
        ofs << *this;
        ofs.close();
    } else {
        throw std::runtime_error("Error opening file " + filename + "!");
    }
}

void EmbeddingHolder::write_to_stdout() const {
    std::string prefix("[OUTPUT]");
    for (const auto &emb : this->emb_matx) {
        std::cout << prefix << emb << '\n';
    }
}

void EmbeddingHolder::update_embedding(int idx, const EmbeddingGradient &gradient, double stepsize) {
    this->emb_matx[idx].update(gradient, stepsize);
}

bool EmbeddingHolder::operator==(const EmbeddingHolder &another) const {
    if (this->get_n_embeddings() != another.emb_matx.size())
        return false;
    for (int i = 0; i < (int)this->emb_matx.size(); ++i) {
        if (this->emb_matx[i] != another.emb_matx[i]) {
            return false;
        }
    }
    return true;
}

int EmbeddingHolder::get_n_embeddings() const { return (int)this->emb_matx.size(); }

Embedding &EmbeddingHolder::operator[](int idx) { return this->emb_matx[idx]; }

int EmbeddingHolder::get_emb_length() const { return this->emb_matx.empty() ? 0 : (*this).emb_matx.at(0).get_length(); }

} // namespace proj1
