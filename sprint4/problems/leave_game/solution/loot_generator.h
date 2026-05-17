#pragma once
#include "model.h"
#include "lost_object.h"
#include <chrono>
#include <vector>
#include <random>

namespace loot_gen {

struct Point {
    double x, y;
};

class LootGenerator {
public:
    struct Config {
        std::chrono::milliseconds period;
        double probability;
    };
    
    LootGenerator(const Config& config);
    
    std::vector<LostObject> Generate(const model::Map& map,
                                     std::chrono::milliseconds time_delta,
                                     int looters_count,
                                     int current_loot_count,
                                     int& next_id);

private:
    Config config_;
    std::chrono::milliseconds accumulator_;
    mutable std::mt19937 rng_;
    std::uniform_real_distribution<double> prob_dist_;
};

} // namespace loot_gen
