#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    try {
        asio::io_context io_context(1);
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        
        std::cout << "Server running on http://localhost:8080" << std::endl;
        
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            
            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);
            
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "MyTestServer");
            res.set(http::field::content_type, "text/plain");
            res.body() = "Hello from Boost.Beast!";
            res.prepare_payload();
            
            http::write(socket, res);
        }
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
