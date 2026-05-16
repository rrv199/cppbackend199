#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "player.h"
#include "collision_detector.h"
#include <vector>

using namespace collision_detector;

TEST_CASE("Player score increases on delivery", "[player]") {
    Player player(5);
    REQUIRE(player.GetScore() == 0);
    
    // Add items with values
    CHECK(player.AddItem(1, 1, 10));
    CHECK(player.AddItem(2, 2, 20));
    CHECK(player.AddItem(3, 3, 30));
    
    REQUIRE(player.GetBagSize() == 3);
    REQUIRE(player.GetScore() == 0);
    
    // Deliver items
    size_t delivered_value = player.DeliverItems();
    REQUIRE(delivered_value == 60); // 10 + 20 + 30
    REQUIRE(player.GetScore() == 60);
    REQUIRE(player.GetBagSize() == 0);
}

TEST_CASE("Player score accumulates over multiple deliveries", "[player]") {
    Player player(5);
    
    player.AddItem(1, 1, 10);
    size_t value1 = player.DeliverItems();
    REQUIRE(value1 == 10);
    REQUIRE(player.GetScore() == 10);
    
    player.AddItem(2, 2, 20);
    player.AddItem(3, 3, 30);
    size_t value2 = player.DeliverItems();
    REQUIRE(value2 == 50);
    REQUIRE(player.GetScore() == 60);
}

TEST_CASE("Player bag capacity with values", "[player]") {
    Player player(2);
    REQUIRE(player.GetBagCapacity() == 2);
    
    CHECK(player.AddItem(1, 1, 10));
    CHECK(player.AddItem(2, 2, 20));
    CHECK(!player.AddItem(3, 3, 30)); // Bag full
    
    REQUIRE(player.GetBagSize() == 2);
    
    size_t delivered = player.DeliverItems();
    REQUIRE(delivered == 30); // 10 + 20
    REQUIRE(player.GetScore() == 30);
}

TEST_CASE("Collision detection with item values", "[collision]") {
    class TestProvider : public ItemGathererProvider {
    public:
        TestProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
            : items_(items), gatherers_(gatherers) {}
        
        size_t ItemsCount() const override { return items_.size(); }
        Item GetItem(size_t idx) const override { return items_[idx]; }
        size_t GatherersCount() const override { return gatherers_.size(); }
        Gatherer GetGatherer(size_t idx) const override { return gatherers_[idx]; }
        
    private:
        std::vector<Item> items_;
        std::vector<Gatherer> gatherers_;
    };
    
    std::vector<Item> items = {
        {{10.0, 0.0}, 0.0, 100, 1},
        {{15.0, 0.0}, 0.0, 200, 2}
    };
    std::vector<Gatherer> gatherers = {{{0.0, 0.0}, {20.0, 0.0}, 0.6}};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 2);
    CHECK(events[0].item_id == 0);
    CHECK(events[1].item_id == 1);
}
