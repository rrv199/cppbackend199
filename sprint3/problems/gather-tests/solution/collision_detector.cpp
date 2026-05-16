#include "collision_detector.h"
#include <algorithm>
#include <cmath>

namespace collision_detector {

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> events;
    const double EPS = 1e-10;
    
    for (size_t g = 0; g < provider.GatherersCount(); ++g) {
        auto gatherer = provider.GetGatherer(g);
        
        // Skip stationary gatherers (no movement)
        if (std::abs(gatherer.start_pos.x - gatherer.end_pos.x) < EPS &&
            std::abs(gatherer.start_pos.y - gatherer.end_pos.y) < EPS) {
            continue;
        }
        
        double dx = gatherer.end_pos.x - gatherer.start_pos.x;
        double dy = gatherer.end_pos.y - gatherer.start_pos.y;
        double len2 = dx * dx + dy * dy;
        
        for (size_t i = 0; i < provider.ItemsCount(); ++i) {
            auto item = provider.GetItem(i);
            
            // Vector from start to item
            double ux = item.position.x - gatherer.start_pos.x;
            double uy = item.position.y - gatherer.start_pos.y;
            
            // Projection onto movement vector
            double dot = ux * dx + uy * dy;
            double t = dot / len2;
            
            // Check if projection falls on segment
            if (t < -EPS || t > 1.0 + EPS) {
                continue;
            }
            
            // Clamp t to segment bounds
            double clamped_t = std::max(0.0, std::min(1.0, t));
            
            // Closest point on the movement segment
            double px = gatherer.start_pos.x + clamped_t * dx;
            double py = gatherer.start_pos.y + clamped_t * dy;
            
            // Distance squared from item to closest point
            double dist2 = (item.position.x - px) * (item.position.x - px) +
                          (item.position.y - py) * (item.position.y - py);
            
            // Convert widths to radii (width is diameter)
            double gatherer_radius = gatherer.width / 2.0;
            double item_radius = item.width / 2.0;
            double sum_radius = gatherer_radius + item_radius;
            double threshold = sum_radius * sum_radius;
            
            if (dist2 <= threshold + EPS) {
                events.push_back({i, g, dist2, clamped_t});
            }
        }
    }
    
    // Sort by time, then by distance for deterministic order
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
