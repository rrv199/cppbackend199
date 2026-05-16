#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "player.h"
#include "collision_detector.h"
#include <vector>

using namespace collision_detector;

TEST_CASE("Player bag capacity", "[player]") {
    Player player(3);
    REQUIRE(player.GetBagCapacity() == 3);
    REQUIRE(player.GetBagSize() == 0);
    
    CHECK(player.AddItem(1, 1));
    CHECK(player.AddItem(2, 2));
    CHECK(player.AddItem(3, 3));
    CHECK(!player.AddItem(4, 4)); // Bag full
    
    REQUIRE(player.GetBagSize() == 3);
    
    size_t delivered = player.DeliverItems();
    REQUIRE(delivered == 3);
    REQUIRE(player.GetBagSize() == 0);
}

TEST_CASE("Collision detection - player picks up item", "[collision]") {
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
    
    std::vector<Item> items = {{{10.0, 0.0}, 0.0}};
    std::vector<Gatherer> gatherers = {{{0.0, 0.0}, {20.0, 0.0}, 0.6}};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 1);
    CHECK(events[0].gatherer_id == 0);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].time == Approx(0.5).margin(1e-9));
}

TEST_CASE("Collision detection - no pickup when far", "[collision]") {
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
    
    std::vector<Item> items = {{{10.0, 10.0}, 0.0}};
    std::vector<Gatherer> gatherers = {{{0.0, 0.0}, {20.0, 0.0}, 0.6}};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.empty());
}

TEST_CASE("Multiple collisions chronological order", "[collision]") {
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
        {{5.0, 0.0}, 0.0},
        {{15.0, 0.0}, 0.0},
        {{10.0, 0.0}, 0.0}
    };
    std::vector<Gatherer> gatherers = {{{0.0, 0.0}, {20.0, 0.0}, 0.6}};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 3);
    for (size_t i = 1; i < events.size(); ++i) {
        CHECK(events[i-1].time <= events[i].time);
    }
}
