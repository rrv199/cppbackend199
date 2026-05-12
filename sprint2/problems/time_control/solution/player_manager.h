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
#include <algorithm>

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
        
        const model::Map* map = nullptr;
        for (const auto& m : game.GetMaps()) {
            if (GetStringFromTagged(m.GetId()) == map_id) {
                map = &m;
                break;
            }
        }
        if (!map) return;
        
        for (auto& [id, player] : it->second) {
            UpdatePlayerPosition(player, time_delta, *map);
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
        
        // Для карты town используем первую горизонтальную дорогу
        for (const auto& road : roads) {
            if (road.IsHorizontal()) {
                auto start = road.GetStart();
                auto end = road.GetEnd();
                double mid_x = (start.x + end.x) / 2.0;
                return {mid_x, static_cast<double>(start.y)};
            }
        }
        
        const auto& road = roads[0];
        auto start = road.GetStart();
        auto end = road.GetEnd();
        if (road.IsHorizontal()) {
            double mid_x = (start.x + end.x) / 2.0;
            return {mid_x, static_cast<double>(start.y)};
        } else {
            double mid_y = (start.y + end.y) / 2.0;
            return {static_cast<double>(start.x), mid_y};
        }
    }
    
    void UpdatePlayerPosition(std::shared_ptr<Player> player, double time_delta, const model::Map& map) {
        const auto& speed = player->GetSpeed();
        if (speed.vx == 0 && speed.vy == 0) return;
        
        Point pos = player->GetPos();
        double new_x = pos.x;
        double new_y = pos.y;
        bool boundary_reached = false;
        
        // Горизонтальное движение
        if (speed.vx != 0) {
            for (const auto& road : map.GetRoads()) {
                if (road.IsHorizontal() && std::abs(pos.y - road.GetStart().y) < 0.5) {
                    auto start = road.GetStart();
                    auto end = road.GetEnd();
                    new_y = static_cast<double>(start.y);
                    new_x = pos.x + speed.vx * time_delta;
                    double min_x = std::min(start.x, end.x) - 0.4;
                    double max_x = std::max(start.x, end.x) + 0.4;
                    
                    if (new_x <= min_x) {
                        new_x = min_x;
                        boundary_reached = true;
                    } else if (new_x >= max_x) {
                        new_x = max_x;
                        boundary_reached = true;
                    }
                    break;
                }
            }
        }
        // Вертикальное движение
        else if (speed.vy != 0) {
            for (const auto& road : map.GetRoads()) {
                if (!road.IsHorizontal() && std::abs(pos.x - road.GetStart().x) < 0.5) {
                    auto start = road.GetStart();
                    auto end = road.GetEnd();
                    new_x = static_cast<double>(start.x);
                    new_y = pos.y + speed.vy * time_delta;
                    double min_y = std::min(start.y, end.y) - 0.4;
                    double max_y = std::max(start.y, end.y) + 0.4;
                    
                    if (new_y <= min_y) {
                        new_y = min_y;
                        boundary_reached = true;
                    } else if (new_y >= max_y) {
                        new_y = max_y;
                        boundary_reached = true;
                    }
                    break;
                }
            }
        }
        
        if (boundary_reached) {
            player->SetSpeed(0, 0);
        }
        
        player->SetPos({new_x, new_y});
    }

    int next_id_ = 0;
    mutable std::mt19937 rng_;
    std::unordered_map<int, std::shared_ptr<Player>> players_;
    std::unordered_map<std::string, std::shared_ptr<Player>> token_to_player_;
    std::unordered_map<std::string, std::unordered_map<int, std::shared_ptr<Player>>> map_players_;
};
