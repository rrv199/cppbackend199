#include <iostream>
#include <vector>
#include <algorithm>
#include "collision_detector.h"
#include "player.h"

using namespace collision_detector;

// Класс для обработки коллизий в игре
class GameCollisionHandler : public ItemGathererProvider {
public:
    struct GameItem {
        Point position;
        double width = 0.0;
        size_t type;
        size_t id;
        bool is_collected = false;
    };
    
    struct GameGatherer {
        Point start_pos;
        Point end_pos;
        double width = 0.6; // Player width
        Player* player;
        size_t id;
    };
    
    struct Base {
        Point position;
        double width = 0.5;
    };
    
    void AddItem(size_t id, size_t type, Point pos) {
        items_.push_back({pos, 0.0, type, id, false});
    }
    
    void AddGatherer(size_t id, Player* player, Point start, Point end) {
        gatherers_.push_back({start, end, 0.6, player, id});
    }
    
    void SetBase(Point pos) {
        base_ = {pos, 0.5};
    }
    
    size_t ItemsCount() const override { return items_.size(); }
    Item GetItem(size_t idx) const override {
        return {items_[idx].position, items_[idx].width};
    }
    
    size_t GatherersCount() const override { return gatherers_.size(); }
    Gatherer GetGatherer(size_t idx) const override {
        return {gatherers_[idx].start_pos, gatherers_[idx].end_pos, gatherers_[idx].width};
    }
    
    void ProcessTick() {
        auto events = FindGatherEvents(*this);
        
        for (const auto& event : events) {
            auto& item = items_[event.item_id];
            auto& gatherer = gatherers_[event.gatherer_id];
            
            if (item.is_collected) continue;
            
            // Check if player reaches base
            double base_dist2 = (gatherer.end_pos.x - base_.position.x) * 
                                (gatherer.end_pos.x - base_.position.x) +
                                (gatherer.end_pos.y - base_.position.y) * 
                                (gatherer.end_pos.y - base_.position.y);
            double base_radius = (gatherer.width + base_.width) / 2.0;
            
            // Deliver items if at base
            if (base_dist2 <= base_radius * base_radius + 1e-10) {
                gatherer.player->DeliverItems();
            }
            
            // Collect item if possible
            if (!item.is_collected) {
                if (gatherer.player->AddItem(item.id, item.type)) {
                    item.is_collected = true;
                }
            }
        }
    }
    
private:
    std::vector<GameItem> items_;
    std::vector<GameGatherer> gatherers_;
    Base base_;
};

int main() {
    std::cout << "Find and Return game module" << std::endl;
    return 0;
}
