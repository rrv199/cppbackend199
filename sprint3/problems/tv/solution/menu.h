#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>

class Menu {
public:
    using Command = std::function<void()>;
    
    void AddCommand(const std::string& name, const std::string& description, Command command);
    void PrintHelp() const;
    bool ExecuteCommand(const std::string& input);

private:
    struct CommandInfo {
        std::string description;
        Command command;
    };
    std::unordered_map<std::string, CommandInfo> commands_;
};
