#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include "http_server.h"
#include "request_handler.h"
#include "json_loader.h"

namespace net = boost::asio;
using namespace std::literals;
namespace sys = boost::system;
namespace fs = std::filesystem;

// Глобальная переменная для режима генерации позиций
bool g_randomize_spawn_points = false;

namespace {

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

struct ProgramOptions {
    std::string config_file;
    std::string www_root;
    std::optional<int> tick_period;
    bool randomize_spawn_points = false;
    bool help = false;
};

ProgramOptions ParseCommandLine(int argc, char* argv[]) {
    namespace po = boost::program_options;
    
    ProgramOptions opts;
    
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("tick-period,t", po::value<int>(), "set tick period in milliseconds")
        ("config-file,c", po::value<std::string>(), "set config file path")
        ("www-root,w", po::value<std::string>(), "set static files root")
        ("randomize-spawn-points", "spawn dogs at random positions");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        opts.help = true;
        std::cout << desc << std::endl;
        return opts;
    }
    
    if (vm.count("config-file")) {
        opts.config_file = vm["config-file"].as<std::string>();
    } else {
        throw std::runtime_error("Config file is required");
    }
    
    if (vm.count("www-root")) {
        opts.www_root = vm["www-root"].as<std::string>();
    } else {
        throw std::runtime_error("www-root is required");
    }
    
    if (vm.count("tick-period")) {
        opts.tick_period = vm["tick-period"].as<int>();
        if (opts.tick_period <= 0) {
            throw std::runtime_error("Tick period must be positive");
        }
    }
    
    if (vm.count("randomize-spawn-points")) {
        opts.randomize_spawn_points = true;
    }
    
    return opts;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        // Parse command line
        auto opts = ParseCommandLine(argc, argv);
        
        if (opts.help) {
            return 0;
        }
        
        // Set global randomize flag
        g_randomize_spawn_points = opts.randomize_spawn_points;
        
        // Load game config
        model::Game game = json_loader::LoadGame(opts.config_file);
        
        // Initialize request handler with options
        http_handler::RequestHandler handler(game, opts.www_root);
        
        // Run server
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        
        auto api_strand = net::make_strand(ioc);
        
        // Setup ticker if tick period is specified
        std::shared_ptr<Ticker> ticker;
        if (opts.tick_period.has_value()) {
            ticker = std::make_shared<Ticker>(
                api_strand,
                std::chrono::milliseconds(*opts.tick_period),
                [&handler](std::chrono::milliseconds delta) {
                    double delta_sec = static_cast<double>(delta.count()) / 1000.0;
                    handler.Tick(delta_sec);
                }
            );
            ticker->Start();
            std::cout << "Server started with tick period: " << *opts.tick_period << " ms" << std::endl;
        } else {
            std::cout << "Server started in test mode (tick via API)" << std::endl;
        }
        
        // Setup signal handling
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code&, int) {
            ioc.stop();
        });
        
        // Serve HTTP
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
