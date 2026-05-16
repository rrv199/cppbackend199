#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace game_serialization {

// Точка на карте
struct Point {
    double x, y;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & x;
        ar & y;
    }
};

// Скорость
struct Speed {
    double vx, vy;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & vx;
        ar & vy;
    }
};

// Направление движения
enum class Direction : uint8_t {
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
};

// Предмет в рюкзаке
struct BagItem {
    size_t id;
    size_t type;
    size_t value;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & id;
        ar & type;
        ar & value;
    }
};

// Собака (игрок)
struct DogState {
    size_t id;
    std::string name;
    Point position;
    Speed speed;
    Direction direction;
    std::vector<BagItem> bag;
    size_t score;
    size_t bag_capacity;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & id;
        ar & name;
        ar & position;
        ar & speed;
        ar & direction;
        ar & bag;
        ar & score;
        ar & bag_capacity;
    }
};

// Потерянный предмет
struct LostObjectState {
    size_t id;
    size_t type;
    size_t value;
    Point position;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & id;
        ar & type;
        ar & value;
        ar & position;
    }
};

// Игрок (пользователь)
struct PlayerState {
    size_t id;
    std::string name;
    std::string token;
    size_t dog_id;
    std::string map_id;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & id;
        ar & name;
        ar & token;
        ar & dog_id;
        ar & map_id;
    }
};

// Полное состояние игры
struct GameState {
    std::vector<DogState> dogs;
    std::vector<LostObjectState> lost_objects;
    std::vector<PlayerState> players;
    size_t next_dog_id;
    size_t next_lost_object_id;
    size_t next_player_id;
    std::chrono::milliseconds game_time;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & dogs;
        ar & lost_objects;
        ar & players;
        ar & next_dog_id;
        ar & next_lost_object_id;
        ar & next_player_id;
        ar & game_time;
    }
};

} // namespace game_serialization
