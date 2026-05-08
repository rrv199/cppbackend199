#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>
#include <random>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        bool is_my_turn = my_initiative;
        
        while (!IsGameEnded()) {
            PrintFields();
            
            if (is_my_turn) {
                // Мой ход
                std::cout << "YOUR TURN! Enter coordinates (e.g., A1): ";
                std::string input;
                std::cin >> input;
                
                auto move = ParseMove(input);
                if (!move.has_value()) {
                    std::cout << "Invalid coordinates! Use format like A1-H8" << std::endl;
                    continue;
                }
                
                auto [x, y] = move.value();
                std::string move_str = MoveToString(move.value());
                
                // Отправляем ход сопернику
                if (!WriteExact(socket, move_str)) {
                    std::cerr << "Failed to send move" << std::endl;
                    break;
                }
                
                // Получаем результат от соперника
                auto result_opt = ReadExact<1>(socket);
                if (!result_opt.has_value()) {
                    std::cerr << "Failed to receive result" << std::endl;
                    break;
                }
                
                SeabattleField::ShotResult result = static_cast<SeabattleField::ShotResult>(result_opt.value()[0]);
                
                // Обрабатываем результат
                if (result == SeabattleField::ShotResult::HIT) {
                    other_field_.MarkHit(x, y);
                    std::cout << "HIT! You get another turn!" << std::endl;
                    // Ход остаётся за нами
                } else if (result == SeabattleField::ShotResult::KILL) {
                    other_field_.MarkKill(x, y);
                    std::cout << "KILL! You destroyed a ship! Another turn!" << std::endl;
                } else {
                    other_field_.MarkMiss(x, y);
                    std::cout << "MISS! Turn passes to opponent." << std::endl;
                    is_my_turn = false;
                }
            } else {
                // Ход соперника
                std::cout << "OPPONENT'S TURN... Waiting for move..." << std::endl;
                
                // Получаем ход от соперника
                auto move_opt = ReadExact<2>(socket);
                if (!move_opt.has_value()) {
                    std::cerr << "Failed to receive move" << std::endl;
                    break;
                }
                
                std::string move_str = move_opt.value();
                std::cout << "Opponent shoots at: " << move_str << std::endl;
                
                auto move = ParseMove(move_str);
                if (!move.has_value()) {
                    std::cerr << "Invalid move received" << std::endl;
                    break;
                }
                
                auto [x, y] = move.value();
                SeabattleField::ShotResult result = my_field_.Shoot(x, y);
                
                // Отправляем результат сопернику
                char result_char = static_cast<char>(result);
                if (!WriteExact(socket, std::string_view(&result_char, 1))) {
                    std::cerr << "Failed to send result" << std::endl;
                    break;
                }
                
                // Обрабатываем результат на своём поле
                if (result == SeabattleField::ShotResult::HIT) {
                    std::cout << "You were hit!" << std::endl;
                    // Противник получает ещё один ход
                } else if (result == SeabattleField::ShotResult::KILL) {
                    std::cout << "Your ship was destroyed!" << std::endl;
                } else {
                    std::cout << "Opponent missed! Your turn!" << std::endl;
                    is_my_turn = true;
                }
            }
        }
        
        // Игра окончена
        std::cout << "\n=== GAME OVER ===" << std::endl;
        PrintFields();
        if (my_field_.IsLoser()) {
            std::cout << "YOU LOST!" << std::endl;
        } else {
            std::cout << "YOU WON!" << std::endl;
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 >= 8) return std::nullopt;
        if (p2 < 0 || p2 >= 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    net::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Server waiting for connection on port " << port << std::endl;
    
    tcp::socket socket(io_context);
    acceptor.accept(socket);
    std::cout << "Client connected!" << std::endl;
    
    SeabattleAgent agent(field);
    agent.StartGame(socket, false);  // сервер не начинает первым
}

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    net::io_context io_context;
    tcp::socket socket(io_context);
    tcp::endpoint endpoint(net::ip::make_address(ip_str), port);
    
    std::cout << "Connecting to server at " << ip_str << ":" << port << std::endl;
    socket.connect(endpoint);
    std::cout << "Connected to server!" << std::endl;
    
    SeabattleAgent agent(field);
    agent.StartGame(socket, true);  // клиент начинает первым
}

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  Server: " << argv[0] << " <seed> <port>" << std::endl;
        std::cout << "  Client: " << argv[0] << " <seed> <ip> <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
