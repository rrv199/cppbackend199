#pragma once
#include "http_server.h"
#include "model.h"
#include <filesystem>

namespace http_handler {

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, std::string static_root)
        : game_(game), static_root_(std::move(static_root)) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    void operator()(http_server::StringRequest&& req, std::function<void(http_server::StringResponse&&)> send);

private:
    model::Game& game_;
    std::filesystem::path static_root_;
};

}  // namespace http_handler
