#pragma once
#include "tagged.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
public:
    struct HorizontalTag {};
    struct VerticalTag {};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_(start), end_({end_x, start.y}) {}

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_(start), end_({start.x, end_y}) {}

    const Point& GetStart() const { return start_; }
    const Point& GetEnd() const { return end_; }

    bool IsHorizontal() const { return start_.y == end_.y; }
    bool IsVertical() const { return start_.x == end_.x; }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    using Id = tagged::Tagged<std::string, Building>;

    explicit Building(Id id, Point pos, Size size) 
        : id_(std::move(id)), position_(pos), size_(size) {}

    const Id& GetId() const { return id_; }
    const Point& GetPosition() const { return position_; }
    const Size& GetSize() const { return size_; }

private:
    Id id_;
    Point position_;
    Size size_;
};

class Office {
public:
    using Id = tagged::Tagged<std::string, Office>;

    Office(Id id, Point pos, Offset offset)
        : id_(std::move(id)), position_(pos), offset_(offset) {}

    const Id& GetId() const { return id_; }
    const Point& GetPosition() const { return position_; }
    const Offset& GetOffset() const { return offset_; }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = tagged::Tagged<std::string, Map>;

    Map(Id id, std::string name) : id_(std::move(id)), name_(std::move(name)), dog_speed_(1.0) {}

    const Id& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    double GetDogSpeed() const { return dog_speed_; }
    void SetDogSpeed(double speed) { dog_speed_ = speed; }

    const std::vector<Road>& GetRoads() const { return roads_; }
    const std::vector<Building>& GetBuildings() const { return buildings_; }
    const std::vector<Office>& GetOffices() const { return offices_; }

    size_t GetLootTypesCount() const { return loot_types_count_; }
    void SetLootTypesCount(size_t count) { loot_types_count_ = count; }

    void AddRoad(Road road) { roads_.push_back(std::move(road)); }
    void AddBuilding(Building building) { buildings_.push_back(std::move(building)); }
    void AddOffice(Office office) { offices_.push_back(std::move(office)); }

private:
    Id id_;
    std::string name_;
    double dog_speed_;
    std::vector<Road> roads_;
    std::vector<Building> buildings_;
    std::vector<Office> offices_;
    size_t loot_types_count_ = 0;
};

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map) { maps_.push_back(std::move(map)); }
    const Maps& GetMaps() const { return maps_; }

private:
    Maps maps_;
};

} // namespace model

