#include "menu.h"
#include <sstream>

void Menu::AddCommand(const std::string& name, const std::string& description, Command command) {
    commands_[name] = {description, command};
}

void Menu::PrintHelp() const {
    std::cout << "Available commands:\n";
    for (const auto& [name, info] : commands_) {
        std::cout << "  " << name << " - " << info.description << "\n";
    }
}

bool Menu::ExecuteCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string command_name;
    iss >> command_name;
    
    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        it->second.command();
        return true;
    }
    return false;
}
