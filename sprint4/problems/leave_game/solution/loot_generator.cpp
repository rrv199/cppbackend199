#include "loot_generator.h"
#include <random>
#include <algorithm>
#include <cmath>

namespace loot_gen {

Point GetRandomPosition(const model::Map& map, std::mt19937& rng) {
    const auto& roads = map.GetRoads();
    if (roads.empty()) return {0.0, 0.0};
    
    std::uniform_int_distribution<size_t> road_dist(0, roads.size() - 1);
    size_t road_idx = road_dist(rng);
    const auto& road = roads[road_idx];
    
    auto start = road.GetStart();
    auto end = road.GetEnd();
    
    std::uniform_real_distribution<double> pos_dist(0.0, 1.0);
    double t = pos_dist(rng);
    
    if (road.IsHorizontal()) {
        double x = start.x + t * (end.x - start.x);
        return {x, static_cast<double>(start.y)};
    } else {
        double y = start.y + t * (end.y - start.y);
        return {static_cast<double>(start.x), y};
    }
}

int GetRandomType(const model::Map& map, std::mt19937& rng) {
    size_t loot_types = map.GetLootTypesCount();
    if (loot_types == 0) return 0;
    std::uniform_int_distribution<int> type_dist(0, static_cast<int>(loot_types) - 1);
    return type_dist(rng);
}

LootGenerator::LootGenerator(const Config& config)
    : config_(config)
    , accumulator_(0)
    , rng_(std::random_device{}())
    , prob_dist_(0.0, 1.0) {
}

std::vector<LostObject> LootGenerator::Generate(const model::Map& map,
                                                 std::chrono::milliseconds time_delta,
                                                 int looters_count,
                                                 int current_loot_count,
                                                 int& next_id) {
    std::vector<LostObject> new_loot;
    
    if (looters_count == 0) {
        accumulator_ = std::chrono::milliseconds(0);
        return new_loot;
    }
    
    accumulator_ += time_delta;
    
    int max_loot = looters_count;
    int needed = std::max(0, max_loot - current_loot_count);
    
    int ticks = static_cast<int>(accumulator_ / config_.period);
    accumulator_ %= config_.period;
    
    for (int i = 0; i < ticks && needed > 0; ++i) {
        if (prob_dist_(rng_) < config_.probability) {
            LostObject loot;
            loot.id = next_id++;
            loot.type = GetRandomType(map, rng_);
            Point pos = GetRandomPosition(map, rng_);
            loot.x = pos.x;
            loot.y = pos.y;
            new_loot.push_back(loot);
            needed--;
        }
    }
    
    return new_loot;
}

} // namespace loot_gen

