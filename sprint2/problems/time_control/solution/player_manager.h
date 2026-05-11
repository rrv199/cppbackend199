#pragma once
#include "player.h"
#include "model.h"
#include <unordered_map>
#include <random>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>
#include <cmath>

class PlayerManager {
public:
    static PlayerManager& Instance() {
        static PlayerManager instance;
        return instance;
    }

    std::pair<int, std::string> CreatePlayer(const std::string& name, const std::string& map_id, const model::Game& game) {
        int id = next_id_++;
        std::string token = GenerateToken();
        
        const model::Map* found_map = nullptr;
        for (const auto& map : game.GetMaps()) {
            if (GetStringFromTagged(map.GetId()) == map_id) {
                found_map = &map;
                break;
            }
        }
        
        Point pos = GetStartPosition(*found_map);
        
        auto player = std::make_shared<Player>(id, name, token, map_id, pos);
        players_[id] = player;
        token_to_player_[token] = player;
        map_players_[map_id][id] = player;
        return {id, token};
    }

    std::shared_ptr<Player> GetPlayerByToken(const std::string& token) const {
        auto it = token_to_player_.find(token);
        if (it != token_to_player_.end()) return it->second;
        return nullptr;
    }

    const std::unordered_map<int, std::shared_ptr<Player>>& GetPlayersOnMap(const std::string& map_id) const {
        static const std::unordered_map<int, std::shared_ptr<Player>> empty;
        auto it = map_players_.find(map_id);
        if (it != map_players_.end()) return it->second;
        return empty;
    }
    
    void UpdatePlayers(const std::string& map_id, double time_delta, const model::Game& game) {
        auto it = map_players_.find(map_id);
        if (it == map_players_.end()) return;
        
        for (auto& [id, player] : it->second) {
            const auto& speed = player->GetSpeed();
            if (speed.vx == 0 && speed.vy == 0) continue;
            
            Point pos = player->GetPos();
            pos.x += speed.vx * time_delta;
            pos.y += speed.vy * time_delta;
            player->SetPos(pos);
        }
    }

private:
    PlayerManager() : rng_(std::random_device{}()) {}

    std::string GenerateToken() {
        std::uniform_int_distribution<int> dist(0, 15);
        std::ostringstream oss;
        for (int i = 0; i < 32; ++i) oss << std::hex << dist(rng_);
        return oss.str();
    }
    
    std::string GetStringFromTagged(const auto& tagged) const {
        return std::string(*tagged);
    }
    
    Point GetStartPosition(const model::Map& map) const {
        const auto& roads = map.GetRoads();
        if (roads.empty()) return {0.0, 0.0};
        
        const auto& first_road = roads[0];
        auto start = first_road.GetStart();
        
        if (first_road.IsHorizontal()) {
            return {static_cast<double>(start.x + 0.5), static_cast<double>(start.y)};
        } else {
            return {static_cast<double>(start.x), static_cast<double>(start.y + 0.5)};
        }
    }

    int next_id_ = 0;
    mutable std::mt19937 rng_;
    std::unordered_map<int, std::shared_ptr<Player>> players_;
    std::unordered_map<std::string, std::shared_ptr<Player>> token_to_player_;
    std::unordered_map<std::string, std::unordered_map<int, std::shared_ptr<Player>>> map_players_;
};
