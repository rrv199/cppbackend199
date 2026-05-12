#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cstring>
#include <boost/asio/signal_set.hpp>
#include "http_server.h"
#include "request_handler.h"
#include "player_manager.h"
extern void SetTickPeriodMode(bool);
#include "json_loader.h"

namespace net = boost::asio;
using namespace std::literals;
namespace sys = boost::system;
namespace fs = std::filesystem;

// Глобальная переменная для режима генерации позиций

struct ProgramOptions {
    std::string config_file;
    std::string www_root;
    int tick_period = -1;
    bool randomize_spawn_points = false;
    bool help = false;
};

void PrintHelp() {
    std::cout << "Allowed options:" << std::endl;
    std::cout << "  -h [ --help ]                     produce help message" << std::endl;
    std::cout << "  -t [ --tick-period ] milliseconds set tick period" << std::endl;
    std::cout << "  -c [ --config-file ] file         set config file path" << std::endl;
    std::cout << "  -w [ --www-root ] dir             set static files root" << std::endl;
    std::cout << "  --randomize-spawn-points          spawn dogs at random positions" << std::endl;
}

ProgramOptions ParseCommandLine(int argc, char* argv[]) {
    ProgramOptions opts;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            opts.help = true;
            return opts;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tick-period") == 0) {
            if (i + 1 < argc) {
                opts.tick_period = std::atoi(argv[++i]);
                if (opts.tick_period <= 0) {
                    throw std::runtime_error("Tick period must be positive");
                }
            } else {
                throw std::runtime_error("Missing value for tick-period");
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config-file") == 0) {
            if (i + 1 < argc) {
                opts.config_file = argv[++i];
            } else {
                throw std::runtime_error("Missing value for config-file");
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--www-root") == 0) {
            if (i + 1 < argc) {
                opts.www_root = argv[++i];
            } else {
                throw std::runtime_error("Missing value for www-root");
            }
        } else if (strcmp(argv[i], "--randomize-spawn-points") == 0) {
            opts.randomize_spawn_points = true;
        }
    }
    
    if (opts.config_file.empty()) {
        throw std::runtime_error("Config file is required");
    }
    if (opts.www_root.empty()) {
        throw std::runtime_error("www-root is required");
    }
    
    return opts;
}

// Ticker class for periodic updates
class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
        : strand_{strand}
        , period_{period}
        , handler_{std::move(handler)} {
    }

    void Start() {
        last_tick_ = Clock::now();
        net::dispatch(strand_, [self = shared_from_this()] {
            self->ScheduleTick();
        });
    }

private:
    void ScheduleTick() {
        timer_.expires_after(period_);
        timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
            self->OnTick(ec);
        });
    }

    void OnTick(sys::error_code ec) {
        using namespace std::chrono;
        if (!ec) {
            auto this_tick = Clock::now();
            auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
            last_tick_ = this_tick;
            try {
                handler_(delta);
            } catch (...) {
            }
            ScheduleTick();
        }
    }

    using Clock = std::chrono::steady_clock;

    Strand strand_;
    std::chrono::milliseconds period_;
    net::steady_timer timer_{strand_};
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};

int main(int argc, char* argv[]) {
    try {
        auto opts = ParseCommandLine(argc, argv);
        
        if (opts.help) {
            PrintHelp();
            return 0;
        }
        
        g_randomize_spawn_points = opts.randomize_spawn_points;
        
        model::Game game = json_loader::LoadGame(opts.config_file);
        http_handler::RequestHandler handler(game, opts.www_root);
        http_handler::SetTickPeriodMode(opts.tick_period > 0);
        http_handler::SetTickPeriodMode(opts.tick_period > 0);
        
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        
        auto api_strand = net::make_strand(ioc);
        
        std::shared_ptr<Ticker> ticker;
        if (opts.tick_period > 0) {
            ticker = std::make_shared<Ticker>(
                api_strand,
                std::chrono::milliseconds(opts.tick_period),
                [&handler](std::chrono::milliseconds delta) {
                    double delta_sec = static_cast<double>(delta.count()) / 1000.0;
                    handler.Tick(delta_sec);
                }
            );
            ticker->Start();
            std::cout << "Server started with tick period: " << opts.tick_period << " ms" << std::endl;
        } else {
            std::cout << "Server started in test mode (tick via API)" << std::endl;
        }
        
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code&, int) {
            ioc.stop();
        });
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, 
            [&handler](auto&& req, auto&& send) {
                handler(std::move(req), std::move(send));
            });
        
        std::cout << "Server has started. Listening on port " << port << std::endl;
        
        ioc.run();
        
        std::cout << "Server has stopped" << std::endl;
        
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
