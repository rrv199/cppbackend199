#include <iostream>
#include <string>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class BookManager {
public:
    BookManager(const std::string& conn_string) {
        // Парсим строку подключения
        std::string dbname;
        std::string host;
        std::string port;
        std::string user;
        std::string password;
        
        // Простой парсинг postgres://user:password@host:port/dbname
        size_t start = conn_string.find("://");
        if (start != std::string::npos) {
            size_t user_end = conn_string.find(":", start + 3);
            size_t pass_end = conn_string.find("@", user_end + 1);
            size_t host_end = conn_string.find(":", pass_end + 1);
            size_t port_end = conn_string.find("/", host_end + 1);
            
            if (user_end != std::string::npos && pass_end != std::string::npos) {
                user = conn_string.substr(start + 3, user_end - start - 3);
                password = conn_string.substr(user_end + 1, pass_end - user_end - 1);
            }
            if (host_end != std::string::npos && port_end != std::string::npos) {
                host = conn_string.substr(pass_end + 1, host_end - pass_end - 1);
                port = conn_string.substr(host_end + 1, port_end - host_end - 1);
                dbname = conn_string.substr(port_end + 1);
            }
        }
        
        // Сначала подключаемся к postgres и создаем базу данных если нужно
        std::string admin_conn = "host=" + host + " port=" + port + " user=" + user + " password=" + password + " dbname=postgres";
        try {
            pqxx::connection admin_conn_obj(admin_conn);
            pqxx::work w(admin_conn_obj);
            w.exec("CREATE DATABASE " + dbname);
            w.commit();
        } catch (...) {
            // База данных уже существует
        }
        
        // Теперь подключаемся к нужной базе данных
        conn_ = std::make_shared<pqxx::connection>(conn_string);
        prepareStatements();
        
        // Создаем таблицу
        try {
            pqxx::work w(*conn_);
            w.exec(
                "CREATE TABLE IF NOT EXISTS books ("
                "id SERIAL PRIMARY KEY, "
                "title varchar(100) NOT NULL, "
                "author varchar(100) NOT NULL, "
                "year integer NOT NULL, "
                "isbn char(13) UNIQUE)");
            w.commit();
        } catch (...) {
            // Игнорируем
        }
    }
    
    bool addBook(const std::string& title, const std::string& author, 
                 int year, const std::optional<std::string>& isbn) {
        try {
            pqxx::work w(*conn_);
            
            if (isbn.has_value()) {
                w.exec_prepared("insert_book", title, author, year, isbn.value());
            } else {
                w.exec_prepared("insert_book", title, author, year, nullptr);
            }
            
            w.commit();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    json getAllBooks() {
        pqxx::read_transaction r(*conn_);
        json result = json::array();
        
        try {
            pqxx::result res = r.exec(
                "SELECT id, title, author, year, isbn FROM books "
                "ORDER BY year DESC, title ASC, author ASC, isbn ASC");
            
            for (const auto& row : res) {
                json book;
                book["id"] = row[0].as<int>();
                book["title"] = row[1].as<std::string>();
                book["author"] = row[2].as<std::string>();
                book["year"] = row[3].as<int>();
                
                if (row[4].is_null()) {
                    book["ISBN"] = nullptr;
                } else {
                    book["ISBN"] = row[4].as<std::string>();
                }
                result.push_back(book);
            }
        } catch (...) {
            // Возвращаем пустой массив
        }
        
        return result;
    }
    
private:
    void prepareStatements() {
        conn_->prepare("insert_book",
            "INSERT INTO books (title, author, year, isbn) "
            "VALUES ($1, $2, $3, $4)");
    }
    
    std::shared_ptr<pqxx::connection> conn_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            return EXIT_FAILURE;
        }
        
        BookManager manager(argv[1]);
        std::string line;
        
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            
            try {
                json request = json::parse(line);
                std::string action = request["action"];
                
                if (action == "add_book") {
                    json payload = request["payload"];
                    std::string title = payload["title"];
                    std::string author = payload["author"];
                    int year = payload["year"];
                    
                    std::optional<std::string> isbn;
                    if (!payload["ISBN"].is_null()) {
                        isbn = payload["ISBN"].get<std::string>();
                    }
                    
                    bool result = manager.addBook(title, author, year, isbn);
                    json response = {{"result", result}};
                    std::cout << response.dump() << std::endl;
                    
                } else if (action == "all_books") {
                    json books = manager.getAllBooks();
                    std::cout << books.dump() << std::endl;
                    
                } else if (action == "exit") {
                    break;
                    
                } else {
                    json response = {{"result", false}};
                    std::cout << response.dump() << std::endl;
                }
                
            } catch (...) {
                json response = {{"result", false}};
                std::cout << response.dump() << std::endl;
            }
        }
        
        return EXIT_SUCCESS;
        
    } catch (...) {
        return EXIT_FAILURE;
    }
}
