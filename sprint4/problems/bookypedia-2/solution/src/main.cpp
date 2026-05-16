#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <pqxx/pqxx>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;

boost::uuids::random_generator uuid_gen;

string generate_uuid() {
    return boost::uuids::to_string(uuid_gen());
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

vector<string> parse_tags(const string& input) {
    vector<string> result;
    stringstream ss(input);
    string tag;
    while (getline(ss, tag, ',')) {
        string trimmed = trim(tag);
        if (trimmed.empty()) continue;
        
        string normalized;
        bool in_space = false;
        for (char c : trimmed) {
            if (c == ' ') {
                if (!in_space) {
                    normalized += ' ';
                    in_space = true;
                }
            } else {
                normalized += c;
                in_space = false;
            }
        }
        if (!normalized.empty() && normalized.back() == ' ') {
            normalized.pop_back();
        }
        if (!normalized.empty()) {
            result.push_back(normalized);
        }
    }
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}

int main() {
    try {
        const char* db_url = getenv("BOOKYPEDIA_DB_URL");
        if (!db_url) {
            cerr << "BOOKYPEDIA_DB_URL environment variable not set" << endl;
            return 1;
        }
        
        // Initialize tables
        {
            pqxx::connection conn(db_url);
            pqxx::work w(conn);
            w.exec("CREATE EXTENSION IF NOT EXISTS pgcrypto;");
            w.exec("CREATE TABLE IF NOT EXISTS authors ("
                   "id uuid PRIMARY KEY, "
                   "name varchar(100) UNIQUE NOT NULL);");
            w.exec("CREATE TABLE IF NOT EXISTS books ("
                   "id uuid PRIMARY KEY, "
                   "author_id uuid NOT NULL REFERENCES authors(id) ON DELETE CASCADE, "
                   "title varchar(100) NOT NULL, "
                   "publication_year integer NOT NULL);");
            w.exec("CREATE TABLE IF NOT EXISTS book_tags ("
                   "book_id uuid NOT NULL REFERENCES books(id) ON DELETE CASCADE, "
                   "tag varchar(30) NOT NULL, "
                   "PRIMARY KEY (book_id, tag));");
            w.commit();
        }
        
        string line;
        while (getline(cin, line)) {
            if (line.empty()) continue;
            
            istringstream iss(line);
            string cmd;
            iss >> cmd;
            
            if (cmd == "AddAuthor") {
                string name = trim(line.substr(cmd.length()));
                if (name.empty()) {
                    cout << "Failed to add author" << endl;
                    continue;
                }
                
                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);
                    w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)",
                                  generate_uuid(), name);
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to add author" << endl;
                }
                
            } else if (cmd == "ShowAuthors") {
                pqxx::connection conn(db_url);
                pqxx::read_transaction r(conn);
                auto res = r.exec("SELECT name FROM authors ORDER BY name");
                int i = 1;
                for (const auto& row : res) {
                    cout << i++ << " " << row[0].as<string>() << endl;
                }
                
            } else if (cmd == "DeleteAuthor") {
                string name = trim(line.substr(cmd.length()));
                
                if (name.empty()) {
                    pqxx::connection conn(db_url);
                    pqxx::read_transaction r(conn);
                    auto res = r.exec("SELECT name FROM authors ORDER BY name");
                    vector<string> authors;
                    for (const auto& row : res) {
                        authors.push_back(row[0].as<string>());
                    }
                    if (authors.empty()) continue;
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i] << endl;
                    }
                    cout << "Enter author # or empty line to cancel" << endl;
                    string choice_line;
                    getline(cin, choice_line);
                    if (choice_line.empty()) continue;
                    int choice = stoi(choice_line);
                    if (choice < 1 || choice > (int)authors.size()) {
                        cout << "Failed to delete author" << endl;
                        continue;
                    }
                    name = authors[choice - 1];
                }
                
                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);
                    auto res = w.exec_params("DELETE FROM authors WHERE name = $1 RETURNING id", name);
                    w.commit();
                    if (res.empty()) {
                        cout << "Failed to delete author" << endl;
                    }
                } catch (const exception& e) {
                    cout << "Failed to delete author" << endl;
                }
                
            } else if (cmd == "EditAuthor") {
                string name = trim(line.substr(cmd.length()));
                
                if (name.empty()) {
                    pqxx::connection conn(db_url);
                    pqxx::read_transaction r(conn);
                    auto res = r.exec("SELECT name FROM authors ORDER BY name");
                    vector<string> authors;
                    for (const auto& row : res) {
                        authors.push_back(row[0].as<string>());
                    }
                    if (authors.empty()) continue;
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i] << endl;
                    }
                    cout << "Enter author # or empty line to cancel" << endl;
                    string choice_line;
                    getline(cin, choice_line);
                    if (choice_line.empty()) continue;
                    int choice = stoi(choice_line);
                    if (choice < 1 || choice > (int)authors.size()) {
                        cout << "Failed to edit author" << endl;
                        continue;
                    }
                    name = authors[choice - 1];
                }
                
                cout << "Enter new name:" << endl;
                string new_name;
                getline(cin, new_name);
                new_name = trim(new_name);
                
                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);
                    auto res = w.exec_params("UPDATE authors SET name = $1 WHERE name = $2 RETURNING id", 
                                              new_name, name);
                    w.commit();
                    if (res.empty()) {
                        cout << "Failed to edit author" << endl;
                    }
                } catch (const exception& e) {
                    cout << "Failed to edit author" << endl;
                }
                
            } else if (cmd == "ShowBooks") {
                pqxx::connection conn(db_url);
                pqxx::read_transaction r(conn);
                auto res = r.exec(
                    "SELECT b.title, a.name, b.publication_year "
                    "FROM books b JOIN authors a ON b.author_id = a.id "
                    "ORDER BY b.title, a.name, b.publication_year");
                int i = 1;
                for (const auto& row : res) {
                    cout << i++ << " " << row[0].as<string>() << " by " 
                         << row[1].as<string>() << ", " << row[2].as<int>() << endl;
                }
                
            } else if (cmd == "ShowBook") {
                string title = trim(line.substr(cmd.length()));
                
                pqxx::connection conn(db_url);
                pqxx::read_transaction r(conn);
                
                if (!title.empty()) {
                    auto res = r.exec_params(
                        "SELECT b.id, b.title, a.name, b.publication_year "
                        "FROM books b JOIN authors a ON b.author_id = a.id "
                        "WHERE b.title = $1 "
                        "ORDER BY b.title, a.name, b.publication_year", title);
                    
                    if (res.empty()) {
                        continue;
                    }
                    
                    if (res.size() == 1) {
                        const auto& row = res[0];
                        cout << "Title: " << row[1].as<string>() << endl;
                        cout << "Author: " << row[2].as<string>() << endl;
                        cout << "Publication year: " << row[3].as<int>() << endl;
                        
                        auto tag_res = r.exec_params("SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag", 
                                                      row[0].as<string>());
                        if (!tag_res.empty()) {
                            cout << "Tags: ";
                            for (size_t j = 0; j < tag_res.size(); ++j) {
                                if (j > 0) cout << ", ";
                                cout << tag_res[j][0].as<string>();
                            }
                            cout << endl;
                        }
                        continue;
                    }
                }
                
                // Show all books for selection
                auto all_books = r.exec(
                    "SELECT b.id, b.title, a.name, b.publication_year "
                    "FROM books b JOIN authors a ON b.author_id = a.id "
                    "ORDER BY b.title, a.name, b.publication_year");
                
                if (all_books.empty()) continue;
                
                cout << "Select book:" << endl;
                vector<string> book_ids;
                for (size_t i = 0; i < all_books.size(); ++i) {
                    const auto& row = all_books[i];
                    cout << i + 1 << " " << row[1].as<string>() << " by " 
                         << row[2].as<string>() << ", " << row[3].as<int>() << endl;
                    book_ids.push_back(row[0].as<string>());
                }
                cout << "Enter the book # or empty line to cancel:" << endl;
                string choice_line;
                getline(cin, choice_line);
                if (choice_line.empty()) continue;
                int choice = stoi(choice_line);
                if (choice < 1 || choice > (int)book_ids.size()) continue;
                
                auto book_res = r.exec_params(
                    "SELECT b.title, a.name, b.publication_year "
                    "FROM books b JOIN authors a ON b.author_id = a.id "
                    "WHERE b.id = $1", book_ids[choice - 1]);
                
                if (!book_res.empty()) {
                    const auto& row = book_res[0];
                    cout << "Title: " << row[0].as<string>() << endl;
                    cout << "Author: " << row[1].as<string>() << endl;
                    cout << "Publication year: " << row[2].as<int>() << endl;
                    
                    auto tag_res = r.exec_params("SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag", 
                                                  book_ids[choice - 1]);
                    if (!tag_res.empty()) {
                        cout << "Tags: ";
                        for (size_t j = 0; j < tag_res.size(); ++j) {
                            if (j > 0) cout << ", ";
                            cout << tag_res[j][0].as<string>();
                        }
                        cout << endl;
                    }
                }
                
            } else if (cmd == "DeleteBook") {
                string title = trim(line.substr(cmd.length()));
                
                pqxx::connection conn(db_url);
                pqxx::read_transaction r(conn);
                
                if (!title.empty()) {
                    auto res = r.exec_params(
                        "SELECT id, title, a.name, publication_year "
                        "FROM books b JOIN authors a ON b.author_id = a.id "
                        "WHERE title = $1", title);
                    
                    if (res.empty()) {
                        cout << "Failed to delete book" << endl;
                        continue;
                    }
                    
                    if (res.size() == 1) {
                        string book_id = res[0][0].as<string>();
                        pqxx::work w(conn);
                        w.exec_params("DELETE FROM books WHERE id = $1", book_id);
                        w.commit();
                        continue;
                    }
                }
                
                // Show all books for selection
                auto all_books = r.exec(
                    "SELECT id, title, a.name, publication_year "
                    "FROM books b JOIN authors a ON b.author_id = a.id "
                    "ORDER BY title, a.name, publication_year");
                
                if (all_books.empty()) {
                    cout << "Failed to delete book" << endl;
                    continue;
                }
                
                cout << "Select book:" << endl;
                vector<string> book_ids;
                for (size_t i = 0; i < all_books.size(); ++i) {
                    const auto& row = all_books[i];
                    cout << i + 1 << " " << row[1].as<string>() << " by " 
                         << row[2].as<string>() << ", " << row[3].as<int>() << endl;
                    book_ids.push_back(row[0].as<string>());
                }
                cout << "Enter the book # or empty line to cancel:" << endl;
                string choice_line;
                getline(cin, choice_line);
                if (choice_line.empty()) continue;
                int choice = stoi(choice_line);
                if (choice < 1 || choice > (int)book_ids.size()) {
                    cout << "Failed to delete book" << endl;
                    continue;
                }
                
                try {
                    pqxx::work w(conn);
                    w.exec_params("DELETE FROM books WHERE id = $1", book_ids[choice - 1]);
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to delete book" << endl;
                }
                
            } else if (cmd == "AddBook") {
                int year;
                iss >> year;
                string title = trim(line.substr(cmd.length() + to_string(year).length()));
                
                if (title.empty()) continue;
                
                cout << "Enter author name or empty line to select from list:" << endl;
                string author_input;
                getline(cin, author_input);
                author_input = trim(author_input);
                
                string author_id;
                
                if (!author_input.empty()) {
                    pqxx::connection conn(db_url);
                    pqxx::read_transaction r(conn);
                    auto res = r.exec_params("SELECT id FROM authors WHERE name = $1", author_input);
                    if (!res.empty()) {
                        author_id = res[0][0].as<string>();
                    } else {
                        cout << "No author found. Do you want to add " << author_input << " (y/n)?" << endl;
                        string answer;
                        getline(cin, answer);
                        answer = trim(answer);
                        if (answer != "y" && answer != "Y") {
                            cout << "Failed to add book" << endl;
                            continue;
                        }
                        author_id = generate_uuid();
                        pqxx::work w(conn);
                        w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)",
                                      author_id, author_input);
                        w.commit();
                    }
                } else {
                    pqxx::connection conn(db_url);
                    pqxx::read_transaction r(conn);
                    auto res = r.exec("SELECT id, name FROM authors ORDER BY name");
                    vector<pair<string, string>> authors;
                    for (const auto& row : res) {
                        authors.push_back({row[0].as<string>(), row[1].as<string>()});
                    }
                    if (authors.empty()) {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i].second << endl;
                    }
                    cout << "Enter author # or empty line to cancel" << endl;
                    string choice_line;
                    getline(cin, choice_line);
                    if (choice_line.empty()) {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                    int choice = stoi(choice_line);
                    if (choice < 1 || choice > (int)authors.size()) {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                    author_id = authors[choice - 1].first;
                }
                
                cout << "Enter tags (comma separated):" << endl;
                string tags_line;
                getline(cin, tags_line);
                auto tags = parse_tags(tags_line);
                
                string book_id = generate_uuid();
                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);
                    w.exec_params("INSERT INTO books (id, author_id, title, publication_year) "
                                  "VALUES ($1, $2, $3, $4)",
                                  book_id, author_id, title, year);
                    for (const auto& tag : tags) {
                        w.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)",
                                      book_id, tag);
                    }
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to add book" << endl;
                }
            }
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
