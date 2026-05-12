#include "request_handler.h"
#include "player_manager.h"
#include <boost/json.hpp>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>

namespace json = boost::json;
using namespace http_server;
namespace fs = std::filesystem;

namespace http_handler {

std::string UrlDecode(std::string_view encoded) {
    std::string result;
    result.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            char hex[3] = {encoded[i + 1], encoded[i + 2], '\0'};
            char ch = static_cast<char>(std::strtol(hex, nullptr, 16));
            result.push_back(ch);
            i += 2;
        } else if (encoded[i] == '+') {
            result.push_back(' ');
        } else {
            result.push_back(encoded[i]);
        }
    }
    return result;
}

std::string GetMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot + 1);
    for (char& c : ext) c = std::tolower(c);
    static const std::unordered_map<std::string, std::string> mime_map = {
        {"htm", "text/html"}, {"html", "text/html"},
        {"css", "text/css"}, {"txt", "text/plain"},
        {"js", "text/javascript"}, {"json", "application/json"},
        {"xml", "application/xml"}, {"png", "image/png"},
        {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"}, {"jpe", "image/jpeg"},
        {"gif", "image/gif"}, {"bmp", "image/bmp"},
        {"ico", "image/vnd.microsoft.icon"},
        {"tiff", "image/tiff"}, {"tif", "image/tiff"},
        {"svg", "image/svg+xml"}, {"svgz", "image/svg+xml"},
        {"mp3", "audio/mpeg"}
    };
    auto it = mime_map.find(ext);
    return it != mime_map.end() ? it->second : "application/octet-stream";
}

StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, bool keep_alive) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, "text/plain");
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse MakeJsonResponse(http::status status, const std::string& body, unsigned http_version, bool keep_alive) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, "application/json");
    response.set(http::field::cache_control, "no-cache");
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

StringResponse MakeFileResponse(const fs::path& file_path, const std::string& mime_type, unsigned http_version, bool keep_alive) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return MakeStringResponse(http::status::not_found, "Not Found", http_version, keep_alive);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    StringResponse response(http::status::ok, http_version);
    response.set(http::field::content_type, mime_type);
    response.body() = content;
    response.content_length(content.size());
    response.keep_alive(keep_alive);
    return response;
}

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

std::string SerializeFullMap(const model::Map& map) {
    json::object result;
    result["id"] = GetStringFromTagged(map.GetId());
    result["name"] = map.GetName();
    json::array roads_array;
    for (const auto& road : map.GetRoads()) {
        json::object road_obj;
        auto start = road.GetStart();
        auto end = road.GetEnd();
        if (road.IsHorizontal()) {
            road_obj["x0"] = start.x;
            road_obj["y0"] = start.y;
            road_obj["x1"] = end.x;
        } else {
            road_obj["x0"] = start.x;
            road_obj["y0"] = start.y;
            road_obj["y1"] = end.y;
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
        office_obj["offsetX"] = offset.dx;
        office_obj["offsetY"] = offset.dy;
        offices_array.push_back(office_obj);
    }
    result["offices"] = offices_array;
    return json::serialize(result);
}

StringResponse HandleJoinGame(const json::value& body, const model::Game& game) {
    try {
        auto obj = body.as_object();
        
        if (!obj.contains("userName") || !obj.contains("mapId")) {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Missing required fields"}}),
                11, true);
        }
        
        std::string user_name = json::value_to<std::string>(obj.at("userName"));
        std::string map_id = json::value_to<std::string>(obj.at("mapId"));
        
        if (user_name.empty()) {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Invalid name"}}),
                11, true);
        }
        
        bool map_found = false;
        for (const auto& map : game.GetMaps()) {
            if (GetStringFromTagged(map.GetId()) == map_id) {
                map_found = true;
                break;
            }
        }
        
        if (!map_found) {
            return MakeJsonResponse(http::status::not_found,
                json::serialize(json::object{{"code", "mapNotFound"}, {"message", "Map not found"}}),
                11, true);
        }
        
        auto [player_id, token] = PlayerManager::Instance().CreatePlayer(user_name, map_id, game);
        
        json::object result;
        result["authToken"] = token;
        result["playerId"] = player_id;
        
        return MakeJsonResponse(http::status::ok, json::serialize(result), 11, true);
    } catch (const std::exception& e) {
        return MakeJsonResponse(http::status::bad_request,
            json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Join game request parse error"}}),
            11, true);
    }
}

StringResponse HandlePlayers(const std::string& token) {
    auto player = PlayerManager::Instance().GetPlayerByToken(token);
    if (!player) {
        return MakeJsonResponse(http::status::unauthorized,
            json::serialize(json::object{{"code", "unknownToken"}, {"message", "Player token has not been found"}}),
            11, true);
    }
    
    auto players = PlayerManager::Instance().GetPlayersOnMap(player->GetMapId());
    
    json::object result;
    for (const auto& [id, p] : players) {
        result[std::to_string(id)] = {{"name", p->GetName()}};
    }
    
    return MakeJsonResponse(http::status::ok, json::serialize(result), 11, true);
}

StringResponse HandleGameState(const std::string& token) {
    auto player = PlayerManager::Instance().GetPlayerByToken(token);
    if (!player) {
        return MakeJsonResponse(http::status::unauthorized,
            json::serialize(json::object{{"code", "unknownToken"}, {"message", "Player token has not been found"}}),
            11, true);
    }
    
    auto players = PlayerManager::Instance().GetPlayersOnMap(player->GetMapId());
    
    json::object players_obj;
    for (const auto& [id, p] : players) {
        json::object player_data;
        player_data["pos"] = json::array({p->GetPos().x, p->GetPos().y});
        player_data["speed"] = json::array({p->GetSpeed().vx, p->GetSpeed().vy});
        player_data["dir"] = DirectionToString(p->GetDir());
        players_obj[std::to_string(id)] = player_data;
    }
    
    json::object result;
    result["players"] = players_obj;
    
    return MakeJsonResponse(http::status::ok, json::serialize(result), 11, true);
}

StringResponse HandlePlayerAction(const json::value& body, const std::string& token, const model::Game& game) {
    auto player = PlayerManager::Instance().GetPlayerByToken(token);
    if (!player) {
        return MakeJsonResponse(http::status::unauthorized,
            json::serialize(json::object{{"code", "unknownToken"}, {"message", "Player token has not been found"}}),
            11, true);
    }

    try {
        auto obj = body.as_object();
        if (!obj.contains("move")) {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Missing move field"}}),
                11, true);
        }

        std::string move = json::value_to<std::string>(obj.at("move"));

        const model::Map* map = nullptr;
        for (const auto& m : game.GetMaps()) {
            if (GetStringFromTagged(m.GetId()) == player->GetMapId()) {
                map = &m;
                break;
            }
        }

        if (!map) {
            return MakeJsonResponse(http::status::internal_server_error,
                json::serialize(json::object{{"code", "internalError"}, {"message", "Map not found"}}),
                11, true);
        }

        double speed_value = map->GetDogSpeed();

        if (move == "L") {
            player->SetSpeed(-speed_value, 0);
            player->SetDirection(Direction::WEST);
        } else if (move == "R") {
            player->SetSpeed(speed_value, 0);
            player->SetDirection(Direction::EAST);
        } else if (move == "U") {
            player->SetSpeed(0, -speed_value);
            player->SetDirection(Direction::NORTH);
        } else if (move == "D") {
            player->SetSpeed(0, speed_value);
            player->SetDirection(Direction::SOUTH);
        } else if (move.empty()) {
            player->SetSpeed(0, 0);
        } else {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Invalid move value"}}),
                11, true);
        }

        return MakeJsonResponse(http::status::ok, "{}", 11, true);

    } catch (const std::exception& e) {
        return MakeJsonResponse(http::status::bad_request,
            json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Failed to parse action"}}),
            11, true);
    }
}

StringResponse HandleGameTick(const json::value& body, const model::Game& game) {
    if (body.is_null()) {
        return MakeJsonResponse(http::status::bad_request,
            json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Missing timeDelta field"}}),
            11, true);
    }
    
    try {
        auto obj = body.as_object();
        if (!obj.contains("timeDelta")) {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Missing timeDelta field"}}),
                11, true);
        }

        int time_delta_ms = 0;
        const auto& delta_val = obj.at("timeDelta");
        if (delta_val.is_int64()) {
            time_delta_ms = static_cast<int>(delta_val.as_int64());
        } else if (delta_val.is_uint64()) {
            time_delta_ms = static_cast<int>(delta_val.as_uint64());
        } else {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "timeDelta must be an integer"}}),
                11, true);
        }

        if (time_delta_ms < 0) {
            return MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "invalidArgument"}, {"message", "timeDelta must be non-negative"}}),
                11, true);
        }

        double time_delta_sec = static_cast<double>(time_delta_ms) / 1000.0;

        for (const auto& map : game.GetMaps()) {
            std::string map_id = GetStringFromTagged(map.GetId());
            PlayerManager::Instance().UpdatePlayers(map_id, time_delta_sec, game);
        }

        return MakeJsonResponse(http::status::ok, "{}", 11, true);
    } catch (const std::exception& e) {
        return MakeJsonResponse(http::status::bad_request,
            json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Failed to parse tick request JSON"}}),
            11, true);
    }
}

void RequestHandler::operator()(StringRequest&& req, std::function<void(StringResponse&&)> send) {
    std::string raw_target(req.target());
    std::string target = raw_target;
    
    if (!target.empty() && target[0] == '/') {
        target = target.substr(1);
    }
    
    if (target.find("api/") == 0) {
        if (target == "api/v1/game/state") {
            if (req.method() == http::verb::get || req.method() == http::verb::head) {
                std::string token;
                auto auth_it = req.find(http::field::authorization);
                if (auth_it == req.end()) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Authorization header missing"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                std::string auth = std::string(auth_it->value());
                if (auth.find("Bearer ") != 0) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid auth format"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                token = auth.substr(7);
                if (token.length() != 32) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid token length"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                auto response = HandleGameState(token);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "GET, HEAD");
                send(std::move(response));
                return;
            }
        }
        else if (target == "api/v1/game/players") {
            if (req.method() == http::verb::get || req.method() == http::verb::head) {
                std::string token;
                auto auth_it = req.find(http::field::authorization);
                if (auth_it == req.end()) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Authorization header missing"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                std::string auth = std::string(auth_it->value());
                if (auth.find("Bearer ") != 0) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid auth format"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                token = auth.substr(7);
                if (token.length() != 32) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid token length"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                auto response = HandlePlayers(token);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "GET, HEAD");
                send(std::move(response));
                return;
            }
        }
        else if (target == "api/v1/game/player/action") {
            if (req.method() == http::verb::post) {
                auto content_type_it = req.find(http::field::content_type);
                if (content_type_it == req.end() || 
                    std::string(content_type_it->value()) != "application/json") {
                    auto response = MakeJsonResponse(http::status::bad_request,
                        json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Invalid content type"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                std::string token;
                auto auth_it = req.find(http::field::authorization);
                if (auth_it == req.end()) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Authorization header is missing"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                std::string auth = std::string(auth_it->value());
                if (auth.find("Bearer ") != 0) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid authorization header format"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                token = auth.substr(7);
                if (token.empty() || token.length() != 32) {
                    auto response = MakeJsonResponse(http::status::unauthorized,
                        json::serialize(json::object{{"code", "invalidToken"}, {"message", "Invalid token format"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                
                auto response = HandlePlayerAction(json::parse(req.body()), token, game_);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Invalid method"}}),
                    11, true);
                response.set(http::field::allow, "POST");
                send(std::move(response));
                return;
            }
        }
        else if (target == "api/v1/game/tick") {
            if (req.method() == http::verb::post) {
                auto content_type_it = req.find(http::field::content_type);
                if (content_type_it == req.end() || 
                    std::string(content_type_it->value()) != "application/json") {
                    auto response = MakeJsonResponse(http::status::bad_request,
                        json::serialize(json::object{{"code", "invalidArgument"}, {"message", "Invalid content type"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
                auto response = HandleGameTick(json::parse(req.body()), game_);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Invalid method"}}),
                    11, true);
                response.set(http::field::allow, "POST");
                send(std::move(response));
                return;
            }
        }
        else if (target == "api/v1/game/join") {
            if (req.method() == http::verb::post) {
                auto response = HandleJoinGame(json::parse(req.body()), game_);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "POST");
                send(std::move(response));
                return;
            }
        }
        else if (target == "api/v1/maps") {
            if (req.method() == http::verb::get) {
                auto response = MakeJsonResponse(http::status::ok, SerializeMaps(game_), 11, true);
                send(std::move(response));
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "GET");
                send(std::move(response));
                return;
            }
        }
        else if (target.find("api/v1/maps/") == 0) {
            if (req.method() == http::verb::get) {
                std::string map_id = target.substr(12);
                const model::Map* found = nullptr;
                for (const auto& map : game_.GetMaps()) {
                    if (GetStringFromTagged(map.GetId()) == map_id) {
                        found = &map;
                        break;
                    }
                }
                if (found) {
                    auto response = MakeJsonResponse(http::status::ok, SerializeFullMap(*found), 11, true);
                    send(std::move(response));
                    return;
                } else {
                    auto response = MakeJsonResponse(http::status::not_found,
                        json::serialize(json::object{{"code", "mapNotFound"}, {"message", "Map not found"}}),
                        11, true);
                    send(std::move(response));
                    return;
                }
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "GET");
                send(std::move(response));
                return;
            }
        }
        else {
            auto response = MakeJsonResponse(http::status::bad_request,
                json::serialize(json::object{{"code", "badRequest"}, {"message", "Bad request"}}),
                11, true);
            send(std::move(response));
            return;
        }
    }
    
    // Static files
    if (req.method() != http::verb::get && req.method() != http::verb::head) {
        send(MakeStringResponse(http::status::method_not_allowed, "Method not allowed", req.version(), req.keep_alive()));
        return;
    }
    
    try {
        std::string decoded = UrlDecode(target);
        if (!decoded.empty() && decoded[0] == '/') {
            decoded = decoded.substr(1);
        }
        if (decoded.empty()) {
            decoded = "index.html";
        }
        
        fs::path file_path = static_root_ / decoded;
        fs::path canonical_path = fs::weakly_canonical(file_path);
        fs::path abs_static_root = fs::weakly_canonical(static_root_);
        
        if (canonical_path.string().find(abs_static_root.string()) != 0) {
            send(MakeStringResponse(http::status::bad_request, "Bad Request", req.version(), req.keep_alive()));
            return;
        }
        
        if (fs::is_directory(canonical_path)) {
            canonical_path /= "index.html";
        }
        
        if (!fs::exists(canonical_path) || !fs::is_regular_file(canonical_path)) {
            send(MakeStringResponse(http::status::not_found, "Not Found", req.version(), req.keep_alive()));
            return;
        }
        
        std::string mime_type = GetMimeType(canonical_path.string());
        StringResponse response = MakeFileResponse(canonical_path, mime_type, req.version(), req.keep_alive());
        
        if (req.method() == http::verb::head) {
            response.body() = "";
        }
        
        send(std::move(response));
        
    } catch (const std::exception& e) {
        send(MakeStringResponse(http::status::internal_server_error, "Internal Error", req.version(), req.keep_alive()));
    }
}

} // namespace http_handler

void RequestHandler::Tick(double time_delta) {
    // Update all maps
    for (const auto& map : game_.GetMaps()) {
        std::string map_id = GetStringFromTagged(map.GetId());
        PlayerManager::Instance().UpdatePlayers(map_id, time_delta, game_);
    }
}

void RequestHandler::Tick(double time_delta) {
    // Update all maps
    for (const auto& map : game_.GetMaps()) {
        std::string map_id = GetStringFromTagged(map.GetId());
        PlayerManager::Instance().UpdatePlayers(map_id, time_delta, game_);
    }
}
