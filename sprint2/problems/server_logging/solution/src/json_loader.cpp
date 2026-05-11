#include <iostream>
#include "json_loader.h"
#include <boost/json.hpp>
#include <fstream>
#include <sstream>

namespace json = boost::json;
using namespace std::literals;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: "s + json_path.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto value = json::parse(buffer.str());
    auto obj = value.as_object();
    
    model::Game game;
    for (auto& map_val : obj.at("maps").as_array()) {
        auto map_obj = map_val.as_object();
        std::string id(map_obj.at("id").as_string().c_str());
        std::string name(map_obj.at("name").as_string().c_str());
        model::Map map(model::Map::Id(id), std::move(name));
        
        // Roads
        for (auto& road_val : map_obj.at("roads").as_array()) {
            auto r = road_val.as_object();
            if (r.contains("x1")) {
                // Горизонтальная
                int x0 = r.at("x0").as_int64();
                int y0 = r.at("y0").as_int64();
                int x1 = r.at("x1").as_int64();
                map.AddRoad({model::Road::HORIZONTAL, {x0, y0}, x1});
            } else {
                // Вертикальная
                int x0 = r.at("x0").as_int64();
                int y0 = r.at("y0").as_int64();
                int y1 = r.at("y1").as_int64();
                map.AddRoad({model::Road::VERTICAL, {x0, y0}, y1});
            }
        }
        
        // Buildings
        if (map_obj.contains("buildings")) {
            for (auto& b_val : map_obj.at("buildings").as_array()) {
                auto b = b_val.as_object();
                int x = b.at("x").as_int64();
                int y = b.at("y").as_int64();
                int w = b.at("w").as_int64();
                int h = b.at("h").as_int64();
                // Создаём Building с Id
                model::Building building(
                    model::Building::Id(std::to_string(x) + "_" + std::to_string(y)),
                    {x, y},
                    {w, h}
                );
                map.AddBuilding(std::move(building));
            }
        }
        
        // Offices
        if (map_obj.contains("offices")) {
            for (auto& o_val : map_obj.at("offices").as_array()) {
                auto o = o_val.as_object();
                std::string office_id(o.at("id").as_string().c_str());
                int x = o.at("x").as_int64();
                int y = o.at("y").as_int64();
                int offsetX = o.at("offsetX").as_int64();
                int offsetY = o.at("offsetY").as_int64();
                model::Office office(
                    model::Office::Id(office_id),
                    {x, y},
                    {offsetX, offsetY}
                );
                map.AddOffice(std::move(office));
            }
        }
        
        game.AddMap(std::move(map));
    }
    return game;
}

} // namespace json_loader
