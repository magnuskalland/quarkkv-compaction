
#include "KeyGenerator.h"

UniformKeyGenerator::UniformKeyGenerator(size_t length) : KeyGenerator(length) {}
std::string UniformKeyGenerator::Generate()
{
    std::uniform_int_distribution<> dist(0, chars_.size() - 1);
    std::string res = "key-";
    for (size_t i = 0; i < length_ - 4; ++i) {
        res += chars_[dist(gen_)];
    }
    return res;
}

ZipfianKeyGenerator::ZipfianKeyGenerator(size_t length) : KeyGenerator(length) {}
std::string ZipfianKeyGenerator::Generate()
{
    std::vector<double> probabilities;
    std::vector<int> weights;
    int sum_weights = 0;

    for (size_t i = 1; i <= chars_.size(); ++i) {
        double weight = 1.0 / (i * i);
        weights.push_back(weight);
        sum_weights += weight;
    }

    for (auto& weight : weights) {
        probabilities.push_back(weight / sum_weights);
    }

    std::vector<double> cumulative_distribution;
    double cumulative = 0.0;
    for (const auto& prob : probabilities) {
        cumulative += prob;
        cumulative_distribution.push_back(cumulative);
    }

    std::string res = "key-";
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < length_ - 4; ++i) {
        double random_value = dist(gen_);
        auto it = std::lower_bound(cumulative_distribution.begin(), cumulative_distribution.end(), random_value);
        size_t index = std::distance(cumulative_distribution.begin(), it);
        res += chars_[index];
    }
    return res;
}