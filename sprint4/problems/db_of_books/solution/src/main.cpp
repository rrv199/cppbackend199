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
        std::string host = "localhost";
        std::string port = "5432";
        std::string user = "postgres";
        std::string password = "";
        std::string dbname;
        
        // Простой парсинг
        std::string temp = conn_string;
        size_t pos = temp.find("://");
        if (pos != std::string::npos) {
            temp = temp.substr(pos + 3);
        }
        
        pos = temp.find('/');
        if (pos != std::string::npos) {
            dbname = temp.substr(pos + 1);
            temp = temp.substr(0, pos);
        }
        
        pos = temp.find('@');
        if (pos != std::string::npos) {
            std::string userpass = temp.substr(0, pos);
            temp = temp.substr(pos + 1);
            pos = userpass.find(':');
            if (pos != std::string::npos) {
                user = userpass.substr(0, pos);
                password = userpass.substr(pos + 1);
            } else {
                user = userpass;
            }
        }
        
        pos = temp.find(':');
        if (pos != std::string::npos) {
            host = temp.substr(0, pos);
            port = temp.substr(pos + 1);
        } else {
            host = temp;
        }
        
        // Создаем строку для подключения к postgres
        std::string admin_conn = "host=" + host + " port=" + port + " user=" + user + " password=" + password + " dbname=postgres";
        
        // Пытаемся создать базу данных
        try {
            pqxx::connection admin_conn_obj(admin_conn);
            pqxx::work w(admin_conn_obj);
            w.exec("CREATE DATABASE " + dbname);
            w.commit();
        } catch (const pqxx::sql_error& e) {
            // База данных уже существует
        } catch (const std::exception& e) {
            // Игнорируем
        }
        
        // Подключаемся к нужной базе
        conn_ = std::make_shared<pqxx::connection>(conn_string);
        prepareStatements();
        createTableIfNotExists();
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
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    json getAllBooks() {
        json result = json::array();
        try {
            pqxx::read_transaction r(*conn_);
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
        } catch (const std::exception& e) {
            // Возвращаем пустой массив
        }
        return result;
    }
    
private:
    void createTableIfNotExists() {
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
        } catch (const std::exception& e) {
            // Игнорируем
        }
    }
    
    void prepareStatements() {
        conn_->prepare("insert_book",
            "INSERT INTO books (title, author, year, isbn) "
            "VALUES ($1, $2, $3, $4)");
    }
    
    std::shared_ptr<pqxx::connection> conn_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    
    try {
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
                    std::cout << json{{"result", result}}.dump() << std::endl;
                    
                } else if (action == "all_books") {
                    json books = manager.getAllBooks();
                    std::cout << books.dump() << std::endl;
                    
                } else if (action == "exit") {
                    break;
                    
                } else {
                    std::cout << json{{"result", false}}.dump() << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cout << json{{"result", false}}.dump() << std::endl;
            }
        }
        
        return EXIT_SUCCESS;
        
    } catch (const std::exception& e) {
        return EXIT_FAILURE;
    }
}
