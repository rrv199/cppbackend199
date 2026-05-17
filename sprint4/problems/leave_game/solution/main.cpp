#include "sdk.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>

#include "json_loader.h"
#include "request_handler.h"
#include "http_server.h"
#include "connection_pool.h"
#include "record_manager.h"

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
        const char* db_url = std::getenv("GAME_DB_URL");
        if (!db_url) {
            std::cerr << "GAME_DB_URL environment variable not set" << std::endl;
            return EXIT_FAILURE;
        }

        model::Game game = json_loader::LoadGame(argv[1]);
        
        // Создаем пул соединений
        ConnectionPool pool(10, [db_url]() {
            return std::make_shared<pqxx::connection>(db_url);
        });
        RecordManager record_manager(pool);
        record_manager.InitTable();

        // Загружаем время бездействия из конфига
        double retirement_time = 60.0;
        // TODO: загрузить из config["dogRetirementTime"]

        fs::path static_path = fs::absolute(fs::path(argv[2]));
        std::cout << "Static files directory: " << static_path.string() << std::endl;

        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) ioc.stop();
        });

        const auto address = net::ip::make_address("0.0.0.0");
        const unsigned short port = 8080;
        tcp::endpoint endpoint(address, port);

        http_handler::RequestHandler handler{game, static_path.string(), record_manager, retirement_time};

        http_server::ServeHttp(ioc, endpoint, [&handler](auto&& req, auto&& send) {
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        std::cout << "Server has started..."sv << std::endl;

        std::vector<std::jthread> workers;
        workers.reserve(num_threads);
        for (unsigned i = 0; i < num_threads; ++i) {
            workers.emplace_back([&ioc] { ioc.run(); });
        }

        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }

    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
