#include "sdk.h"
#include <iostream>
#include "connection_pool.h"
#include "record_manager.h"

int main() {
    const char* db_url = std::getenv("GAME_DB_URL");
    if (!db_url) {
        std::cerr << "GAME_DB_URL not set" << std::endl;
        return 1;
    }
    
    ConnectionPool pool(1, [db_url]() {
        return std::make_shared<pqxx::connection>(db_url);
    });
    RecordManager rm(pool);
    rm.InitTable();
    
    std::cout << "Server started" << std::endl;
    return 0;
}

