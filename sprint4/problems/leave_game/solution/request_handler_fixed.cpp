#include "request_handler.h"
#include <boost/json.hpp>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <iostream>

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

http::status HandleApiRequest(const std::string& target, const model::Game& game, std::string& body) {
    if (target == "/api/v1/maps") {
        body = SerializeMaps(game);
        return http::status::ok;
    }
    
    if (target.find("/api/v1/maps/") == 0) {
        std::string map_id = target.substr(14);
        for (const auto& map : game.GetMaps()) {
            if (GetStringFromTagged(map.GetId()) == map_id) {
                body = json::serialize(json::object{
                    {"id", GetStringFromTagged(map.GetId())},
                    {"name", map.GetName()}
                });
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
    std::string target(req.target());
    
    // Обработка API
    if (target.find("/api/") == 0) {
        std::string body;
        http::status status;
        
        if (req.method() != http::verb::get) {
            body = json::serialize(json::object{{"code", "badRequest"}, {"message", "Bad request"}});
            status = http::status::bad_request;
        } else {
            status = HandleApiRequest(target, game_, body);
        }
        
        StringResponse response(status, req.version());
        response.set(http::field::content_type, "application/json");
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(req.keep_alive());
        send(std::move(response));
        return;
    }
    
    // Обработка статических файлов
    if (req.method() != http::verb::get && req.method() != http::verb::head) {
        send(MakeStringResponse(http::status::method_not_allowed, "Method not allowed", req.version(), req.keep_alive()));
        return;
    }
    
    try {
        std::string decoded = UrlDecode(target);
        if (!decoded.empty() && decoded[0] == '/') {
            decoded = decoded.substr(1);
        }
        
        fs::path file_path = static_root_ / decoded;
        fs::path canonical_path = fs::weakly_canonical(file_path);
        
        // Исправленная проверка безопасности
        fs::path abs_static_root = fs::weakly_canonical(static_root_);
        std::string cp_str = canonical_path.string();
        std::string sr_str = abs_static_root.string();
        
        std::cerr << "DEBUG: abs_static_root = " << sr_str << std::endl;
        std::cerr << "DEBUG: canonical_path = " << cp_str << std::endl;
        
        if (cp_str.find(sr_str) != 0) {
            std::cerr << "DEBUG: Security violation!" << std::endl;
            send(MakeStringResponse(http::status::bad_request, "Bad Request", req.version(), req.keep_alive()));
            return;
        }
        
        // Если это директория - добавляем index.html
        if (fs::is_directory(canonical_path)) {
            canonical_path /= "index.html";
            cp_str = canonical_path.string();
        }
        
        // Проверка существования файла
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
        std::cerr << "DEBUG: Exception: " << e.what() << std::endl;
        send(MakeStringResponse(http::status::internal_server_error, "Internal Error", req.version(), req.keep_alive()));
    }
}

} // namespace http_handler
