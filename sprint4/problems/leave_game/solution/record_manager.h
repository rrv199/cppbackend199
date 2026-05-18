#pragma once
#include <string>
#include <vector>
#include "connection_pool.h"

class RecordManager {
public:
    RecordManager(ConnectionPool& pool) : pool_(pool) {}

    void InitTable() {
        auto conn = pool_.GetConnection();
        pqxx::work w(*conn);
        w.exec(R"(CREATE TABLE IF NOT EXISTS retired_players (
            id SERIAL PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            score INTEGER NOT NULL,
            play_time DOUBLE PRECISION NOT NULL
        );)");
        w.commit();
    }

    void AddRecord(const std::string& name, int score, double play_time) {
        auto conn = pool_.GetConnection();
        pqxx::work w(*conn);
        w.exec_params("INSERT INTO retired_players (name, score, play_time) VALUES ($1, $2, $3)",
                      name, score, play_time);
        w.commit();
    }

private:
    ConnectionPool& pool_;
};

