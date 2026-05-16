#include "collision_detector.h"
#include <algorithm>

namespace collision_detector {

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> events;
    const double EPS = 1e-10;
    
    for (size_t g = 0; g < provider.GatherersCount(); ++g) {
        auto gatherer = provider.GetGatherer(g);
        
        if (std::abs(gatherer.start_pos.x - gatherer.end_pos.x) < EPS &&
            std::abs(gatherer.start_pos.y - gatherer.end_pos.y) < EPS) {
            continue;
        }
        
        double dx = gatherer.end_pos.x - gatherer.start_pos.x;
        double dy = gatherer.end_pos.y - gatherer.start_pos.y;
        double len2 = dx * dx + dy * dy;
        
        double gatherer_radius = gatherer.width / 2.0;
        
        for (size_t i = 0; i < provider.ItemsCount(); ++i) {
            auto item = provider.GetItem(i);
            double item_radius = item.width / 2.0;
            double sum_radius = gatherer_radius + item_radius;
            
            double ux = item.position.x - gatherer.start_pos.x;
            double uy = item.position.y - gatherer.start_pos.y;
            
            double dot = ux * dx + uy * dy;
            double t = dot / len2;
            
            if (t < -EPS || t > 1.0 + EPS) {
                continue;
            }
            
            double clamped_t = std::max(0.0, std::min(1.0, t));
            
            double px = gatherer.start_pos.x + clamped_t * dx;
            double py = gatherer.start_pos.y + clamped_t * dy;
            
            double dist2 = (item.position.x - px) * (item.position.x - px) +
                          (item.position.y - py) * (item.position.y - py);
            
            double threshold = sum_radius * sum_radius;
            
            if (dist2 <= threshold + EPS) {
                events.push_back({i, g, dist2, clamped_t});
            }
        }
    }
    
    std::sort(events.begin(), events.end(),
        [](const GatheringEvent& a, const GatheringEvent& b) {
            if (std::abs(a.time - b.time) > 1e-10) {
                return a.time < b.time;
            }
            return a.sq_distance < b.sq_distance;
        });
    
    return events;
}

} // namespace collision_detector
