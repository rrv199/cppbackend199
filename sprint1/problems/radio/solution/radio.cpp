#include "audio.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <boost/asio.hpp>

namespace net = boost::asio;
using net::ip::udp;

constexpr size_t MAX_FRAMES = 65000;
constexpr auto RECORD_DURATION = std::chrono::milliseconds(1500);

void StartServer(uint16_t port) {
    try {
        net::io_context io;
        udp::socket socket(io, udp::endpoint(udp::v4(), port));

        std::cout << "Server started on port " << port 
                  << ". Waiting for audio..." << std::endl;

        Player player(ma_format_u8, 1);
        size_t frame_size = player.GetFrameSize();

        while (true) {
            std::vector<char> buffer(MAX_FRAMES * frame_size);
            udp::endpoint client_endpoint;

            size_t received = socket.receive_from(net::buffer(buffer), client_endpoint);

            if (received > 0) {
                size_t frames = received / frame_size;
                std::cout << "Received " << frames << " frames from "
                          << client_endpoint.address().to_string() << ":"
                          << client_endpoint.port() << std::endl;

                buffer.resize(received);
                player.PlayBuffer(buffer.data(), frames, RECORD_DURATION);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void StartClient(uint16_t port) {
    try {
        net::io_context io;
        udp::socket socket(io, udp::endpoint(udp::v4(), 0));

        Recorder recorder(ma_format_u8, 1);
        size_t frame_size = recorder.GetFrameSize();

        std::cout << "Client started." << std::endl;

        while (true) {
            std::string server_ip;
            std::cout << "Enter server IP address (or 'quit' to exit): ";
            std::getline(std::cin, server_ip);

            if (server_ip == "quit" || server_ip == "q") {
                break;
            }

            if (server_ip.empty()) {
                server_ip = "127.0.0.1";
            }

            std::cout << "Press ENTER to record and send audio..." << std::endl;
            std::cin.get();

            auto rec_result = recorder.Record(MAX_FRAMES, RECORD_DURATION);
            std::cout << "Recording done. Frames: " << rec_result.frames << std::endl;

            if (rec_result.frames > 0) {
                size_t bytes_to_send = rec_result.frames * frame_size;

                std::cout << "Sending " << rec_result.frames << " frames ("
                          << bytes_to_send << " bytes) to " << server_ip << ":" << port << std::endl;

                udp::endpoint server_endpoint(net::ip::make_address(server_ip), port);
                socket.send_to(net::buffer(rec_result.data.data(), bytes_to_send), server_endpoint);

                std::cout << "Data sent!" << std::endl;
            } else {
                std::cout << "No audio recorded." << std::endl;
            }

            std::cin.ignore();
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <client|server> <port>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    if (mode == "server") {
        StartServer(port);
    } else if (mode == "client") {
        StartClient(port);
    } else {
        std::cerr << "Mode must be 'client' or 'server'" << std::endl;
        return 1;
    }

    return 0;
}
