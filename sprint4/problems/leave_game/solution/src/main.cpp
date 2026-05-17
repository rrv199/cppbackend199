#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "game_server.h"

using json = nlohmann::json;

int main() {
    try {
        const char* db_url = std::getenv("GAME_DB_URL");
        if (!db_url) {
            std::cerr << "GAME_DB_URL environment variable not set" << std::endl;
            return 1;
        }
        
        GameServer server(db_url);
        
        // Пример использования (в реальном сервере здесь будет HTTP обработка)
        std::cout << "Game server started with database connection pool" << std::endl;
        
        // Демонстрация работы
        server.AddPlayer("token1", "Rex");
        server.AddPlayer("token2", "Buddy");
        
        // Симуляция игровых тиков
        for (int i = 0; i < 10; ++i) {
            server.ProcessTick(1.0);
        }
        
        // Получение рекордов
        auto records = server.GetRecords(0, 10);
        std::cout << "Records: " << json(records).dump() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
