#include <iostream>
#include <string>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class BookManager {
public:
    BookManager(const std::string& conn_string) 
        : conn_(std::make_shared<pqxx::connection>(conn_string)) {
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
        } catch (const pqxx::sql_error& e) {
            // Если таблицы нет, создаем и пробуем снова
            if (std::string(e.what()).find("does not exist") != std::string::npos) {
                createTableIfNotExists();
                return addBook(title, author, year, isbn);
            }
            return false;
        } catch (const std::exception& e) {
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
        } catch (const pqxx::sql_error& e) {
            // Таблицы нет - создаем и пробуем снова
            if (std::string(e.what()).find("does not exist") != std::string::npos) {
                createTableIfNotExists();
                return getAllBooks();
            }
        }
        
        return result;
    }
    
private:
    void prepareStatements() {
        conn_->prepare("create_table",
            "CREATE TABLE IF NOT EXISTS books ("
            "id SERIAL PRIMARY KEY, "
            "title varchar(100) NOT NULL, "
            "author varchar(100) NOT NULL, "
            "year integer NOT NULL, "
            "isbn char(13) UNIQUE)");
        
        conn_->prepare("insert_book",
            "INSERT INTO books (title, author, year, isbn) "
            "VALUES ($1, $2, $3, $4)");
    }
    
    void createTableIfNotExists() {
        try {
            pqxx::work w(*conn_);
            w.exec_prepared("create_table");
            w.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error creating table: " << e.what() << std::endl;
        }
    }
    
    std::shared_ptr<pqxx::connection> conn_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <connection-string>" << std::endl;
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
                
            } catch (const json::parse_error& e) {
                continue;
            }
        }
        
        return EXIT_SUCCESS;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
