#pragma once

#include "request_handler.h"
#include "logger.h"
#include <chrono>

template<typename Handler>
class LoggingRequestHandler {
public:
    explicit LoggingRequestHandler(Handler& handler) : handler_(handler) {}
    
    void operator()(http_server::StringRequest&& req, std::function<void(http_server::StringResponse&&)> send) {
        // Логируем запрос
        LogRequest(req);
        
        auto start_time = std::chrono::steady_clock::now();
        
        // Вызываем оригинальный обработчик
        handler_(std::move(req), [this, send, start_time](http_server::StringResponse&& res) {
            // Логируем ответ
            LogResponse(res, start_time);
            send(std::move(res));
        });
    }
    
private:
    void LogRequest(const http_server::StringRequest& req) {
        json::value data = {
            {"ip", req.base().at("Host").to_string()},  // Упрощённо, нужно реальный IP
            {"URI", req.target()},
            {"method", std::string(req.method_string())}
        };
        Logger::Log(data, logging::trivial::info, "request received");
    }
    
    void LogResponse(const http_server::StringResponse& res, std::chrono::steady_clock::time_point start) {
        auto end = std::chrono::steady_clock::now();
        auto response_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        json::value data = {
            {"response_time", static_cast<boost::json::int64>(response_time)},
            {"code", res.result_int()}
        };
        
        auto content_type = res.find("Content-Type");
        if (content_type != res.end()) {
            data.as_object()["content_type"] = std::string(content_type->value());
        } else {
            data.as_object()["content_type"] = nullptr;
        }
        
        Logger::Log(data, logging::trivial::info, "response sent");
    }
    
    Handler& handler_;
};
