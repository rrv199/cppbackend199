#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "collision_detector.h"
#include <vector>
#include <cmath>

using namespace collision_detector;

// Helper class to provide test data
class TestProvider : public ItemGathererProvider {
public:
    TestProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
        : items_(std::move(items)), gatherers_(std::move(gatherers)) {}
    
    size_t ItemsCount() const override { return items_.size(); }
    Item GetItem(size_t idx) const override { return items_[idx]; }
    size_t GatherersCount() const override { return gatherers_.size(); }
    Gatherer GetGatherer(size_t idx) const override { return gatherers_[idx]; }
    
private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

// String maker for Catch2
namespace Catch {
template<>
struct StringMaker<GatheringEvent> {
    static std::string convert(GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << "," << value.item_id 
            << "," << value.sq_distance << "," << value.time << ")";
        return tmp.str();
    }
};
}

TEST_CASE("FindGatherEvents detects collision when gatherer passes through item", "[collision_detector]") {
    Item item;
    item.position = {10.0, 0.0};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 1);
    CHECK(events[0].gatherer_id == 0);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].sq_distance == Approx(0.0).margin(1e-9));
    CHECK(events[0].time == Approx(0.5).margin(1e-9));
}

TEST_CASE("FindGatherEvents detects no collision when item is far away", "[collision_detector]") {
    Item item;
    item.position = {100.0, 100.0};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.empty());
}

TEST_CASE("FindGatherEvents detects collision when item is close to path", "[collision_detector]") {
    Item item;
    item.position = {10.0, 0.9};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 1);
}

TEST_CASE("FindGatherEvents detects no collision when item is just outside range", "[collision_detector]") {
    Item item;
    item.position = {10.0, 1.1};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.empty());
}

TEST_CASE("FindGatherEvents detects multiple collisions", "[collision_detector]") {
    std::vector<Item> items(3);
    items[0].position = {10.0, 0.0};
    items[0].width = 1.0;
    items[1].position = {10.0, 2.0};
    items[1].width = 1.0;
    items[2].position = {15.0, 0.0};
    items[2].width = 1.0;
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 2);
}

TEST_CASE("FindGatherEvents returns events in chronological order", "[collision_detector]") {
    std::vector<Item> items(3);
    items[0].position = {5.0, 0.0};
    items[0].width = 1.0;
    items[1].position = {15.0, 0.0};
    items[1].width = 1.0;
    items[2].position = {10.0, 0.0};
    items[2].width = 1.0;
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 3);
    for (size_t i = 1; i < events.size(); ++i) {
        CHECK(events[i-1].time <= events[i].time);
    }
}

TEST_CASE("FindGatherEvents handles stationary gatherer", "[collision_detector]") {
    Item item;
    item.position = {10.0, 0.0};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {5.0, 0.0};
    gatherer.end_pos = {5.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.empty());
}

TEST_CASE("FindGatherEvents handles vertical movement", "[collision_detector]") {
    Item item;
    item.position = {0.0, 10.0};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {0.0, 20.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    TestProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    REQUIRE(events.size() == 1);
    CHECK(events[0].time == Approx(0.5).margin(1e-9));
}
