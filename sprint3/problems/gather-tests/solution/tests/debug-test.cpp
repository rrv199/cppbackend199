#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "collision_detector.h"
#include <iostream>

using namespace collision_detector;

TEST_CASE("Debug item just outside range", "[debug]") {
    Item item;
    item.position = {10.0, 1.1};
    item.width = 1.0;
    std::vector<Item> items = {item};
    
    Gatherer gatherer;
    gatherer.start_pos = {0.0, 0.0};
    gatherer.end_pos = {20.0, 0.0};
    gatherer.width = 1.0;
    std::vector<Gatherer> gatherers = {gatherer};
    
    class DebugProvider : public ItemGathererProvider {
    public:
        DebugProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
            : items_(items), gatherers_(gatherers) {}
        size_t ItemsCount() const override { return items_.size(); }
        Item GetItem(size_t idx) const override { return items_[idx]; }
        size_t GatherersCount() const override { return gatherers_.size(); }
        Gatherer GetGatherer(size_t idx) const override { return gatherers_[idx]; }
    private:
        std::vector<Item> items_;
        std::vector<Gatherer> gatherers_;
    };
    
    DebugProvider provider(items, gatherers);
    auto events = FindGatherEvents(provider);
    
    std::cout << "Number of events: " << events.size() << std::endl;
    for (auto& e : events) {
        std::cout << "Event: gatherer=" << e.gatherer_id 
                  << " item=" << e.item_id
                  << " dist2=" << e.sq_distance
                  << " time=" << e.time << std::endl;
    }
    
    // Calculate theoretical values
    double dx = 20.0, dy = 0.0;
    double len2 = 400.0;
    double ux = 10.0, uy = 1.1;
    double dot = ux * dx + uy * dy;
    double t = dot / len2;
    double px = 0 + t * dx;
    double py = 0 + t * dy;
    double dist2 = (ux - px)*(ux - px) + (uy - py)*(uy - py);
    double sum_width = 2.0;
    
    std::cout << "Theoretical: t=" << t << " dist2=" << dist2 << " threshold=" << sum_width*sum_width << std::endl;
    
    REQUIRE(events.empty());
}
