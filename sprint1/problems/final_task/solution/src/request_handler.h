#pragma once
#include "http_server.h"
#include "model.h"

namespace http_handler {

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game) : game_(game) {}

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    void operator()(http_server::StringRequest&& req, std::function<void(http_server::StringResponse&&)> send);

private:
    model::Game& game_;
};

}  // namespace http_handler
