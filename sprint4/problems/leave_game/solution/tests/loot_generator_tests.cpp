#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../loot_generator.h"
#include "../model.h"
#include <chrono>

using namespace loot_gen;

TEST_CASE("LootGenerator basic functionality", "[loot]") {
    LootGenerator::Config config;
    config.period = std::chrono::milliseconds(1000);
    config.probability = 1.0;
    
    LootGenerator generator(config);
    
    model::Map map(model::Map::Id{"test"}, "Test Map");
    map.SetLootTypesCount(3);
    
    map.AddRoad(model::Road(model::Road::HorizontalTag{}, {0, 0}, 10));
    
    int next_id = 0;
    auto loot = generator.Generate(map, std::chrono::milliseconds(5000), 5, 0, next_id);
    
    CHECK(loot.size() <= 5);
}

TEST_CASE("LootGenerator respects looters count", "[loot]") {
    LootGenerator::Config config;
    config.period = std::chrono::milliseconds(1000);
    config.probability = 1.0;
    
    LootGenerator generator(config);
    
    model::Map map(model::Map::Id{"test"}, "Test Map");
    map.SetLootTypesCount(3);
    map.AddRoad(model::Road(model::Road::HorizontalTag{}, {0, 0}, 10));
    
    int next_id = 0;
    
    SECTION("Zero looters produces no loot") {
        auto loot = generator.Generate(map, std::chrono::milliseconds(10000), 0, 0, next_id);
        CHECK(loot.empty());
    }
    
    SECTION("Loot limited by looters count") {
        auto loot = generator.Generate(map, std::chrono::milliseconds(10000), 3, 0, next_id);
        CHECK(loot.size() <= 3);
    }
}

TEST_CASE("LootGenerator respects max loot limit", "[loot]") {
    LootGenerator::Config config;
    config.period = std::chrono::milliseconds(1000);
    config.probability = 1.0;
    
    LootGenerator generator(config);
    
    model::Map map(model::Map::Id{"test"}, "Test Map");
    map.SetLootTypesCount(3);
    map.AddRoad(model::Road(model::Road::HorizontalTag{}, {0, 0}, 10));
    
    int next_id = 0;
    
    auto loot = generator.Generate(map, std::chrono::milliseconds(5000), 5, 5, next_id);
    CHECK(loot.empty());
}

TEST_CASE("LootGenerator type range", "[loot]") {
    LootGenerator::Config config;
    config.period = std::chrono::milliseconds(1);
    config.probability = 1.0;
    
    LootGenerator generator(config);
    
    model::Map map(model::Map::Id{"test"}, "Test Map");
    map.SetLootTypesCount(5);
    map.AddRoad(model::Road(model::Road::HorizontalTag{}, {0, 0}, 10));
    
    int next_id = 0;
    
    auto loot = generator.Generate(map, std::chrono::milliseconds(10000), 50, 0, next_id);
    
    for (const auto& item : loot) {
        CHECK(item.type >= 0);
        CHECK(item.type < 5);
    }
}

TEST_CASE("LootGenerator random positions", "[loot]") {
    LootGenerator::Config config;
    config.period = std::chrono::milliseconds(1);
    config.probability = 1.0;
    
    LootGenerator generator(config);
    
    model::Map map(model::Map::Id{"test"}, "Test Map");
    map.SetLootTypesCount(1);
    
    map.AddRoad(model::Road(model::Road::HorizontalTag{}, {0, 0}, 100));
    map.AddRoad(model::Road(model::Road::VerticalTag{}, {0, 0}, 100));
    
    int next_id = 0;
    
    auto loot = generator.Generate(map, std::chrono::milliseconds(10000), 100, 0, next_id);
    
    CHECK(loot.size() > 0);
}
