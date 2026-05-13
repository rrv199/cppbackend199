#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "loot_generator.h"
#include <random>
#include <chrono>

TEST_CASE("LootGenerator generates loot", "[loot_generator]") {
    LootGenerator generator(std::chrono::milliseconds(1000), 1.0);
    std::mt19937 rng(42);
    
    int loot = generator.Generate(rng, 5, std::chrono::milliseconds(5000));
    REQUIRE(loot >= 0);
    REQUIRE(loot <= 5);
}

TEST_CASE("LootGenerator zero probability", "[loot_generator]") {
    LootGenerator generator(std::chrono::milliseconds(1000), 0.0);
    std::mt19937 rng(42);
    
    int loot = generator.Generate(rng, 5, std::chrono::milliseconds(5000));
    REQUIRE(loot == 0);
}

TEST_CASE("LootGenerator respects max looters", "[loot_generator]") {
    LootGenerator generator(std::chrono::milliseconds(100), 1.0);
    std::mt19937 rng(42);
    
    int loot = generator.Generate(rng, 3, std::chrono::milliseconds(1000));
    REQUIRE(loot <= 3);
}
