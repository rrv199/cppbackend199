#include "json_loader.h"
#include <iostream>

namespace json_loader {

SimpleGame LoadGame(const std::string& config_path) {
    std::cout << "Loading game from: " << config_path << std::endl;
    return SimpleGame();
}

} // namespace json_loader
