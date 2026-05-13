#pragma once
#include <string>

namespace json_loader {

// Простая структура для демонстрации
class SimpleGame {
public:
    int GetMapsCount() const { return 1; }
};

SimpleGame LoadGame(const std::string& config_path);

} // namespace json_loader
