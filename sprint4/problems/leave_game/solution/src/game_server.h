#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include "connection_pool.h"
#include "record_manager.h"
#include "player.h"

class GameServer {
public:
    GameServer(const std::string& db_url, double retirement_time = 60.0) 
        : pool_(10, [db_url]() { 
            return std::make_shared<pqxx::connection>(db_url); 
        }),
        record_manager_(pool_),
        retirement_time_(retirement_time) {
        record_manager_.InitTable();
    }
    
    void ProcessTick(double delta_time) {
        std::lock_guard lock(mutex_);
        std::vector<std::string> retired_players;
        
        for (auto& [token, player] : players_) {
            player.Update(delta_time, player.GetSpeed());
            
            if (player.IsIdle(retirement_time_)) {
                retired_players.push_back(token);
                record_manager_.AddRecord(
                    player.GetName(),
                    player.GetScore(),
                    player.GetTotalPlayTime()
                );
            }
        }
        
        for (const auto& token : retired_players) {
            players_.erase(token);
        }
    }
    
    void AddPlayer(const std::string& token, const std::string& name) {
        std::lock_guard lock(mutex_);
        players_.emplace(token, Player(name));
    }
    
    std::vector<Record> GetRecords(int start, int max_items) {
        return record_manager_.GetRecords(start, max_items);
    }
    
private:
    ConnectionPool pool_;
    RecordManager record_manager_;
    std::unordered_map<std::string, Player> players_;
    double retirement_time_;
    std::mutex mutex_;
};
