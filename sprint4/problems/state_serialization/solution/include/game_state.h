#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

namespace game_serialization {

struct Point {
    double x, y;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & x & y;
    }
};

struct Speed {
    double vx, vy;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & vx & vy;
    }
};

enum class Direction : uint8_t {
    NONE = 0, UP = 1, DOWN = 2, LEFT = 3, RIGHT = 4
};

struct BagItem {
    size_t id, type, value;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & id & type & value;
    }
};

struct DogState {
    size_t id, score, bag_capacity;
    std::string name;
    Point position;
    Speed speed;
    Direction direction;
    std::vector<BagItem> bag;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & id & name & position & speed & direction & bag & score & bag_capacity;
    }
};

struct LostObjectState {
    size_t id, type, value;
    Point position;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & id & type & value & position;
    }
};

struct PlayerState {
    size_t id, dog_id;
    std::string name, token, map_id;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & id & name & token & dog_id & map_id;
    }
};

struct GameState {
    std::vector<DogState> dogs;
    std::vector<LostObjectState> lost_objects;
    std::vector<PlayerState> players;
    size_t next_dog_id = 0;
    size_t next_lost_object_id = 0;
    size_t next_player_id = 0;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar & dogs & lost_objects & players;
        ar & next_dog_id & next_lost_object_id & next_player_id;
    }
};

} // namespace game_serialization
