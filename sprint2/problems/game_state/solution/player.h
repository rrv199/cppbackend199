#pragma once
#include <string>

struct Point { double x, y; };
struct Speed { double vx, vy; };
enum class Direction { NORTH, SOUTH, WEST, EAST };

inline std::string DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::NORTH: return "U";
        case Direction::SOUTH: return "D";
        case Direction::WEST: return "L";
        case Direction::EAST: return "R";
        default: return "U";
    }
}

class Player {
public:
    Player(int id, std::string name, const std::string& token, const std::string& map_id, const Point& pos)
        : id_(id), name_(std::move(name)), token_(token), map_id_(map_id), pos_(pos), speed_{0.0, 0.0}, dir_(Direction::NORTH) {}
    int GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const std::string& GetToken() const { return token_; }
    const std::string& GetMapId() const { return map_id_; }
    const Point& GetPos() const { return pos_; }
    const Speed& GetSpeed() const { return speed_; }
    Direction GetDir() const { return dir_; }
private:
    int id_;
    std::string name_;
    std::string token_;
    std::string map_id_;
    Point pos_;
    Speed speed_;
    Direction dir_;
};
