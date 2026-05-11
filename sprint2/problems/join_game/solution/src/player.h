#pragma once

#include <string>

class Player {
public:
    Player(int id, std::string name, const std::string& token, const std::string& map_id)
        : id_(id)
        , name_(std::move(name))
        , token_(token)
        , map_id_(map_id) {}

    int GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const std::string& GetToken() const { return token_; }
    const std::string& GetMapId() const { return map_id_; }

private:
    int id_;
    std::string name_;
    std::string token_;
    std::string map_id_;
};
