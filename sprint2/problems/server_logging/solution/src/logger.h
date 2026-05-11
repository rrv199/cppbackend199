#pragma once

#include <boost/json.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <mutex>
#include <string>

namespace json = boost::json;

class Logger {
public:
    static Logger& Instance() {
        static Logger instance;
        return instance;
    }
    
    void Log(const json::value& data, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        json::object obj;
        obj["timestamp"] = GetTimestamp();
        obj["message"] = message;
        obj["data"] = data;
        
        std::cout << json::serialize(obj) << std::endl;
    }
    
    void LogRequest(const std::string& ip, const std::string& uri, const std::string& method) {
        json::value data = {
            {"ip", ip},
            {"URI", uri},
            {"method", method}
        };
        Log(data, "request received");
    }
    
    void LogResponse(int response_time, int code, const std::string& content_type) {
        json::value data = {
            {"response_time", response_time},
            {"code", code}
        };
        if (!content_type.empty()) {
            data.as_object()["content_type"] = content_type;
        } else {
            data.as_object()["content_type"] = nullptr;
        }
        Log(data, "response sent");
    }
    
    void LogError(int code, const std::string& text, const std::string& where) {
        json::value data = {
            {"code", code},
            {"text", text},
            {"where", where}
        };
        Log(data, "error");
    }
    
    void LogServerStarted(int port, const std::string& address) {
        json::value data = {
            {"port", port},
            {"address", address}
        };
        Log(data, "server started");
    }
    
    void LogServerExited(int code, const std::string& exception = "") {
        json::value data = {{"code", code}};
        if (!exception.empty()) {
            data.as_object()["exception"] = exception;
        }
        Log(data, "server exited");
    }
    
private:
    Logger() = default;
    
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()) % 1000000;
        
        std::tm tm;
        localtime_r(&time_t, &tm);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
            << "." << std::setfill('0') << std::setw(6) << ms.count();
        return oss.str();
    }
    
    std::mutex mutex_;
};
