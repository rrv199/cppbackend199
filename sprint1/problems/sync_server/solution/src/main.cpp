#ifdef WIN32
#include <sdkddkver.h>
#endif
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include <optional>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;

// Синонимы для удобства
using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;

// Структура для типов контента
struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
};

// Создаёт ответ с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, 
                                   unsigned http_version, bool keep_alive,
                                   std::string_view content_type = ContentType::TEXT_HTML) {
    StringResponse response(status, http_version);
    response.set(http::field::content_type, content_type);
    response.body() = body;
    response.content_length(body.size());
    response.keep_alive(keep_alive);
    return response;
}

// Чтение запроса
std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer) {
    beast::error_code ec;
    StringRequest req;
    http::read(socket, buffer, req, ec);

    if (ec == http::error::end_of_stream) {
        return std::nullopt;
    }
    if (ec) {
        throw std::runtime_error("Failed to read request: "s + ec.message());
    }
    return req;
}

// Вывод информации о запросе
void DumpRequest(const StringRequest& req) {
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    for (const auto& header : req) {
        std::cout << "  " << header.name_string() << ": " << header.value() << std::endl;
    }
}

// Обработка запроса
StringResponse HandleRequest(StringRequest&& req) {
    std::string target(req.target());
    
    // Удаляем ведущий слэш
    if (!target.empty() && target[0] == '/') {
        target = target.substr(1);
    }
    
    // Проверяем метод запроса
    if (req.method() == http::verb::get) {
        std::string body = "Hello, " + target;
        return MakeStringResponse(http::status::ok, body, req.version(), req.keep_alive());
    } 
    else if (req.method() == http::verb::head) {
        std::string body = "Hello, " + target;
        StringResponse response = MakeStringResponse(http::status::ok, "", req.version(), req.keep_alive());
        response.content_length(body.size());
        return response;
    }
    else {
        // Method Not Allowed
        StringResponse response = MakeStringResponse(
            http::status::method_not_allowed, 
            "Invalid method", 
            req.version(), 
            req.keep_alive()
        );
        response.set(http::field::allow, "GET, HEAD");
        return response;
    }
}

// Обработка соединения
void HandleConnection(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        
        while (auto request = ReadRequest(socket, buffer)) {
            DumpRequest(*request);
            StringResponse response = HandleRequest(*std::move(request));
            http::write(socket, response);
            if (response.need_eof()) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
    }
    
    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    try {
        net::io_context ioc;
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr unsigned short port = 8080;
        
        tcp::acceptor acceptor(ioc, {address, port});
        
        std::cout << "Server has started..." << std::endl;
        
        while (true) {
            tcp::socket socket(ioc);
            acceptor.accept(socket);
            
            std::thread t(
                [](tcp::socket sock) {
                    HandleConnection(std::move(sock));
                },
                std::move(socket)
            );
            t.detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
