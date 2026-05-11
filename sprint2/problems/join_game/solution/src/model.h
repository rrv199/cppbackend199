#pragma once
#include "tagged.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    int x, y;
};

struct Size {
    int width, height;
};

class Road {
public:
    struct HorizontalTag {};
    struct VerticalTag {};
    static constexpr HorizontalTag HORIZONTAL{};
    static constexpr VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_(start), end_(end_x, start.y) {}

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_(start), end_(start.x, end_y) {}

    Point GetStart() const { return start_; }
    Point GetEnd() const { return end_; }
    bool IsHorizontal() const { return start_.y == end_.y; }
    bool IsVertical() const { return start_.x == end_.x; }
    Coord GetEndCoord() const { return IsHorizontal() ? end_.x : end_.y; }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    using Id = util::Tagged<std::string, Building>;
    explicit Building(Id id, Point pos, Size size) : id_(id), position_(pos), size_(size) {}
    const Id& GetId() const { return id_; }
    Point GetPosition() const { return position_; }
    Size GetSize() const { return size_; }

private:
    Id id_;
    Point position_;
    Size size_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;
    Office(Id id, Point pos, Point offset) : id_(id), position_(pos), offset_(offset) {}
    const Id& GetId() const { return id_; }
    Point GetPosition() const { return position_; }
    Point GetOffset() const { return offset_; }

private:
    Id id_;
    Point position_;
    Point offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    Map(Id id, std::string name) : id_(id), name_(std::move(name)) {}
    const Id& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    void AddRoad(Road road) { roads_.push_back(std::move(road)); }
    void AddBuilding(Building building) { buildings_.push_back(std::move(building)); }
    void AddOffice(Office office) { offices_.push_back(std::move(office)); }
    const std::vector<Road>& GetRoads() const { return roads_; }
    const std::vector<Building>& GetBuildings() const { return buildings_; }
    const std::vector<Office>& GetOffices() const { return offices_; }

private:
    Id id_;
    std::string name_;
    std::vector<Road> roads_;
    std::vector<Building> buildings_;
    std::vector<Office> offices_;
};

class Game {
public:
    void AddMap(Map map) { maps_.push_back(std::move(map)); }
    const std::vector<Map>& GetMaps() const { return maps_; }
    const Map* FindMap(const Map::Id& id) const {
        for (const auto& map : maps_) {
            if (map.GetId() == id) return &map;
        }
        return nullptr;
    }

private:
    std::vector<Map> maps_;
};

}  // namespace model
