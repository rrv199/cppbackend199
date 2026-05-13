#pragma once
#include <chrono>
#include <random>

class LootGenerator {
public:
    LootGenerator(std::chrono::milliseconds period, double probability)
        : period_(period), probability_(probability), generator_(std::random_device()()) {}
    
    template<typename RandomGenerator>
    int Generate(RandomGenerator& rng, int looters_count, std::chrono::milliseconds time_delta) {
        time_accumulated_ += time_delta;
        int generated = 0;
        while (time_accumulated_ >= period_ && generated < looters_count) {
            std::bernoulli_distribution dist(probability_);
            if (dist(rng)) {
                generated++;
            }
            time_accumulated_ -= period_;
        }
        return generated;
    }
    
private:
    std::chrono::milliseconds period_;
    double probability_;
    std::chrono::milliseconds time_accumulated_{0};
    std::mt19937 generator_;
};
