#pragma once

#include <string>
#include <vector>
#include "connection_pool.h"

struct Record {
    std::string name;
    int score;
    double play_time;
};

class RecordManager {
public:
    RecordManager(ConnectionPool& pool) : pool_(pool) {}

    void InitTable() {
        auto conn = pool_.GetConnection();
        pqxx::work w(*conn);
        w.exec(R"(
            CREATE TABLE IF NOT EXISTS retired_players (
                id SERIAL PRIMARY KEY,
                name VARCHAR(100) NOT NULL,
                score INTEGER NOT NULL,
                play_time DOUBLE PRECISION NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            CREATE INDEX IF NOT EXISTS idx_retired_players_score_time 
                ON retired_players (score DESC, play_time ASC, name ASC);
        )");
        w.commit();
    }

    void AddRecord(const std::string& name, int score, double play_time) {
        auto conn = pool_.GetConnection();
        pqxx::work w(*conn);
        w.exec_params(
            "INSERT INTO retired_players (name, score, play_time) VALUES ($1, $2, $3)",
            name, score, play_time);
        w.commit();
    }

    std::vector<Record> GetRecords(int start, int max_items) {
        if (max_items > 100) {
            throw std::runtime_error("maxItems cannot exceed 100");
        }
        
        auto conn = pool_.GetConnection();
        pqxx::read_transaction r(*conn);
        
        auto res = r.exec_params(
            "SELECT name, score, play_time FROM retired_players "
            "ORDER BY score DESC, play_time ASC, name ASC "
            "LIMIT $1 OFFSET $2",
            max_items, start);
        
        std::vector<Record> records;
        for (const auto& row : res) {
            records.push_back({
                row[0].as<std::string>(),
                row[1].as<int>(),
                row[2].as<double>()
            });
        }
        return records;
    }

private:
    ConnectionPool& pool_;
};
