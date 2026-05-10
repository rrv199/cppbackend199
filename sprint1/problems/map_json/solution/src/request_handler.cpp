#include "request_handler.h"
#include <boost/json.hpp>

namespace json = boost::json;
using namespace http_server;

namespace http_handler {

std::string GetStringFromTagged(const auto& tagged) {
    return std::string(*tagged);
}

std::string SerializeMaps(const model::Game& game) {
    json::array maps_array;
    for (const auto& map : game.GetMaps()) {
        json::object map_obj;
        map_obj["id"] = GetStringFromTagged(map.GetId());
        map_obj["name"] = map.GetName();
        maps_array.push_back(map_obj);
    }
    return json::serialize(maps_array);
}

std::string SerializeMap(const model::Map& map) {
    json::object result;
    result["id"] = GetStringFromTagged(map.GetId());
    result["name"] = map.GetName();
    
    json::array roads_array;
    for (const auto& road : map.GetRoads()) {
        json::object road_obj;
        auto start = road.GetStart();
        if (road.IsHorizontal()) {
            road_obj["x0"] = start.x;
            road_obj["y0"] = start.y;
            road_obj["x1"] = road.GetEndCoord();
        } else {
            road_obj["x0"] = start.x;
            road_obj["y0"] = start.y;
            road_obj["y1"] = road.GetEndCoord();
        }
        roads_array.push_back(road_obj);
    }
    result["roads"] = roads_array;
    
    json::array buildings_array;
    for (const auto& building : map.GetBuildings()) {
        json::object building_obj;
        auto pos = building.GetPosition();
        auto size = building.GetSize();
        building_obj["x"] = pos.x;
        building_obj["y"] = pos.y;
        building_obj["w"] = size.width;
        building_obj["h"] = size.height;
        buildings_array.push_back(building_obj);
    }
    result["buildings"] = buildings_array;
    
    json::array offices_array;
    for (const auto& office : map.GetOffices()) {
        json::object office_obj;
        office_obj["id"] = GetStringFromTagged(office.GetId());
        auto pos = office.GetPosition();
        office_obj["x"] = pos.x;
        office_obj["y"] = pos.y;
        auto offset = office.GetOffset();
        office_obj["offsetX"] = offset.x;
        office_obj["offsetY"] = offset.y;
        offices_array.push_back(office_obj);
    }
    result["offices"] = offices_array;
    
    return json::serialize(result);
}

http::status HandleApiRequest(const std::string& target, const model::Game& game, std::string& body) {
    const std::string API_PREFIX = "/api/v1/";
    if (target.find(API_PREFIX) != 0) {
        body = json::serialize(json::object{{"code", "badRequest"}, {"message", "Bad request"}});
        return http::status::bad_request;
    }
    
    std::string path = target.substr(API_PREFIX.length());
    
    if (path == "maps") {
        body = SerializeMaps(game);
        return http::status::ok;
    }
    
    if (path.find("maps/") == 0) {
        std::string map_id = path.substr(5);
        for (const auto& map : game.GetMaps()) {
            if (GetStringFromTagged(map.GetId()) == map_id) {
                body = SerializeMap(map);
                return http::status::ok;
            }
        }
        body = json::serialize(json::object{{"code", "mapNotFound"}, {"message", "Map not found"}});
        return http::status::not_found;
    }
    
    body = json::serialize(json::object{{"code", "badRequest"}, {"message", "Bad request"}});
    return http::status::bad_request;
}

void RequestHandler::operator()(StringRequest&& req, std::function<void(StringResponse&&)> send) {
    std::string body;
    http::status status;
    
    if (req.method() != http::verb::get) {
        body = json::serialize(json::object{{"code", "badRequest"}, {"message", "Bad request"}});
        status = http::status::bad_request;
    } else {
        std::string target(req.target());
        status = HandleApiRequest(target, game_, body);
    }
    
    StringResponse response(status, req.version());
    response.set(http::field::content_type, "application/json");
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(req.keep_alive());
    
    send(std::move(response));
}

} // namespace http_handler
