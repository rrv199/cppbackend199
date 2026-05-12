#pragma once
#include "http_server.h"
#include "model.h"
#include <filesystem>
#include <functional>

namespace http_handler {
void SetTickPeriodMode(bool enabled);
void SetTickPeriodMode(bool enabled);

class RequestHandler {
public:
    explicit RequestHandler(const model::Game& game, const std::filesystem::path& static_root, bool randomize_spawn = false)
        : game_(game)
        , static_root_(static_root)
        , randomize_spawn_(randomize_spawn) {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    void operator()(http_server::StringRequest&& req, std::function<void(http_server::StringResponse&&)> send);
    
    void Tick(double time_delta);

private:
    const model::Game& game_;
    std::filesystem::path static_root_;
    bool randomize_spawn_;
};

} // namespace http_handler

void SetTickPeriodMode(bool enabled);

void SetTickPeriodMode(bool enabled);

void SetTickPeriodMode(bool enabled);

// Глобальная переменная для tick period
extern bool g_has_tick_period;
void SetTickPeriodMode(bool enabled);

namespace http_handler {
    extern bool g_has_tick_period;
    void SetTickPeriodMode(bool enabled);
}
