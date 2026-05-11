#include "sdk.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>

#include "json_loader.h"
#include "request_handler.h"
#include "logging_request_handler.h"
#include "http_server.h"
#include "logger.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
using tcp = net::ip::tcp;
namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: game_server <config-file> <static-dir>"sv << std::endl;
        return EXIT_FAILURE;
    }
    
    try {
        // Инициализируем логгер
        Logger::Init();
        
        model::Game game = json_loader::LoadGame(argv[1]);
        
        fs::path static_path = fs::absolute(fs::path(argv[2]));
        
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                json::value data = {{"code", 0}};
                Logger::Log(data, logging::trivial::info, "server exited");
                ioc.stop();
            }
        });
        
        const auto address = net::ip::make_address("0.0.0.0");
        const unsigned short port = 8080;
        tcp::endpoint endpoint(address, port);
        
        // Логируем запуск сервера
        json::value start_data = {
            {"port", port},
            {"address", address.to_string()}
        };
        Logger::Log(start_data, logging::trivial::info, "server started");
        
        // Создаём обработчик с логированием
        http_handler::RequestHandler handler{game, static_path.string()};
        LoggingRequestHandler logging_handler{handler};
        
        http_server::ServeHttp(ioc, endpoint, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });
        
        std::vector<std::jthread> workers;
        workers.reserve(num_threads);
        for (unsigned i = 0; i < num_threads; ++i) {
            workers.emplace_back([&ioc] { ioc.run(); });
        }
        
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        
    } catch (const std::exception& ex) {
        json::value error_data = {
            {"code", EXIT_FAILURE},
            {"exception", ex.what()}
        };
        Logger::Log(error_data, logging::trivial::error, "server exited");
        return EXIT_FAILURE;
    }
    
    return 0;
}
