#include "json_loader.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace json = boost::json;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file");
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    auto value = json::parse(content);
    auto obj = value.as_object();
    
    model::Game game;
    
    // Read default dog speed
    double default_dog_speed = 1.0;
    if (obj.contains("defaultDogSpeed")) {
        default_dog_speed = obj.at("defaultDogSpeed").as_double();
    }
    
    // Load maps
    if (!obj.contains("maps")) {
        throw std::runtime_error("No maps in config");
    }
    
    for (const auto& map_value : obj.at("maps").as_array()) {
        auto map_obj = map_value.as_object();
        
        if (!map_obj.contains("id")) {
            throw std::runtime_error("Map has no id");
        }
        if (!map_obj.contains("name")) {
            throw std::runtime_error("Map has no name");
        }
        
        std::string id = json::value_to<std::string>(map_obj.at("id"));
        std::string name = json::value_to<std::string>(map_obj.at("name"));
        
        model::Map map(model::Map::Id{std::move(id)}, std::move(name));
        
        // Set dog speed for this map
        double dog_speed = default_dog_speed;
        if (map_obj.contains("dogSpeed")) {
            dog_speed = map_obj.at("dogSpeed").as_double();
        }
        map.SetDogSpeed(dog_speed);
        
        // Load roads
        if (map_obj.contains("roads")) {
            for (const auto& road_value : map_obj.at("roads").as_array()) {
                auto road_obj = road_value.as_object();
                model::Point start;
                start.x = road_obj.at("x0").as_int64();
                start.y = road_obj.at("y0").as_int64();
                
                if (road_obj.contains("x1")) {
                    // Horizontal road
                    model::Coord end_x = road_obj.at("x1").as_int64();
                    map.AddRoad(model::Road(model::Road::HorizontalTag{}, start, end_x));
                } else if (road_obj.contains("y1")) {
                    // Vertical road
                    model::Coord end_y = road_obj.at("y1").as_int64();
                    map.AddRoad(model::Road(model::Road::VerticalTag{}, start, end_y));
                }
            }
        }
        
        // Load buildings
        if (map_obj.contains("buildings")) {
            for (const auto& building_value : map_obj.at("buildings").as_array()) {
                auto building_obj = building_value.as_object();
                std::string building_id = std::to_string(building_obj.at("x").as_int64()) + 
                                         "_" + std::to_string(building_obj.at("y").as_int64());
                model::Point pos;
                model::Size size;
                pos.x = building_obj.at("x").as_int64();
                pos.y = building_obj.at("y").as_int64();
                size.width = building_obj.at("w").as_int64();
                size.height = building_obj.at("h").as_int64();
                map.AddBuilding(model::Building(
                    model::Building::Id{std::move(building_id)},
                    pos, size));
            }
        }
        
        // Load offices
        if (map_obj.contains("offices")) {
            for (const auto& office_value : map_obj.at("offices").as_array()) {
                auto office_obj = office_value.as_object();
                std::string office_id = json::value_to<std::string>(office_obj.at("id"));
                model::Point pos;
                model::Offset offset;
                pos.x = office_obj.at("x").as_int64();
                pos.y = office_obj.at("y").as_int64();
                offset.dx = office_obj.at("offsetX").as_int64();
                offset.dy = office_obj.at("offsetY").as_int64();
                map.AddOffice(model::Office(
                    model::Office::Id{std::move(office_id)},
                    pos, offset));
            }
        }
        
        game.AddMap(std::move(map));
    }
    
    return game;
}

} // namespace json_loader
