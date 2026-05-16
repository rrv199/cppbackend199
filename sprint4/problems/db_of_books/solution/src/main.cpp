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
        // Извлекаем имя базы данных из строки подключения
        std::string dbname;
        std::string base_conn_string;
        
        // Парсим строку подключения
        size_t last_slash = conn_string.rfind('/');
        if (last_slash != std::string::npos) {
            dbname = conn_string.substr(last_slash + 1);
            base_conn_string = conn_string.substr(0, last_slash + 1) + "postgres";
        } else {
            dbname = "postgres";
            base_conn_string = conn_string;
        }
        
        // Проверяем, существует ли база данных
        bool db_exists = false;
        try {
            pqxx::connection test_conn(conn_string);
            db_exists = true;
        } catch (const pqxx::broken_connection& e) {
            db_exists = false;
        }
        
        if (!db_exists) {
            // Создаем базу данных
            try {
                pqxx::connection admin_conn(base_conn_string);
                pqxx::work w(admin_conn);
                w.exec("CREATE DATABASE " + dbname);
                w.commit();
            } catch (const std::exception& e) {
                // Не удалось создать базу данных
            }
        }
        
        // Подключаемся к базе данных
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
        } catch (const pqxx::sql_error& e) {
            return false;
        } catch (...) {
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
                
            } catch (const std::exception& e) {
                json response = {{"result", false}};
                std::cout << response.dump() << std::endl;
            }
        }
        
        return EXIT_SUCCESS;
        
    } catch (const std::exception& e) {
        return EXIT_FAILURE;
    }
}
