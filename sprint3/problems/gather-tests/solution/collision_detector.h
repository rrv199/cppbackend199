#pragma once
#include <vector>
#include <cmath>

namespace collision_detector {

struct Item {
    struct Point { double x, y; } position;
    double width;
};

struct Gatherer {
    struct Point { double x, y; } start_pos, end_pos;
    double width;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;
public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

} // namespace collision_detector
