#pragma once

#include "player.h"
#include <unordered_map>
#include <random>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>

class PlayerManager {
public:
    static PlayerManager& Instance() {
        static PlayerManager instance;
        return instance;
    }

    std::pair<int, std::string> CreatePlayer(const std::string& name, const std::string& map_id) {
        int id = next_id_++;
        std::string token = GenerateToken();
        auto player = std::make_shared<Player>(id, name, token, map_id);
        players_[id] = player;
        token_to_player_[token] = player;
        map_players_[map_id][id] = player;
        return {id, token};
    }

    std::shared_ptr<Player> GetPlayerByToken(const std::string& token) const {
        auto it = token_to_player_.find(token);
        if (it != token_to_player_.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::unordered_map<int, std::shared_ptr<Player>>& GetPlayersOnMap(const std::string& map_id) const {
        static const std::unordered_map<int, std::shared_ptr<Player>> empty;
        auto it = map_players_.find(map_id);
        if (it != map_players_.end()) {
            return it->second;
        }
        return empty;
    }

private:
    PlayerManager() : rng_(std::random_device{}()) {}

    std::string GenerateToken() {
        std::uniform_int_distribution<int> dist(0, 15);
        std::ostringstream oss;
        for (int i = 0; i < 32; ++i) {
            oss << std::hex << dist(rng_);
        }
        return oss.str();
    }

    int next_id_ = 0;
    std::mt19937 rng_;
    std::unordered_map<int, std::shared_ptr<Player>> players_;
    std::unordered_map<std::string, std::shared_ptr<Player>> token_to_player_;
    std::unordered_map<std::string, std::unordered_map<int, std::shared_ptr<Player>>> map_players_;
};
