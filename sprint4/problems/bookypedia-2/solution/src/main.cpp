#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <pqxx/pqxx>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;

boost::uuids::random_generator uuid_gen;

string generate_uuid() {
    return boost::uuids::to_string(uuid_gen());
}

void ensure_tables(pqxx::connection& conn) {
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

vector<pair<string, string>> get_authors(pqxx::connection& conn) {
    pqxx::read_transaction r(conn);
    auto res = r.exec("SELECT id, name FROM authors ORDER BY name");
    vector<pair<string, string>> authors;
    for (const auto& row : res) {
        authors.push_back({row[0].as<string>(), row[1].as<string>()});
    }
    return authors;
}

vector<tuple<string, string, string, int>> get_books(pqxx::connection& conn) {
    pqxx::read_transaction r(conn);
    auto res = r.exec(
        "SELECT b.id, b.title, a.name, b.publication_year "
        "FROM books b JOIN authors a ON b.author_id = a.id "
        "ORDER BY b.title, a.name, b.publication_year");
    vector<tuple<string, string, string, int>> books;
    for (const auto& row : res) {
        books.push_back({row[0].as<string>(), row[1].as<string>(), 
                         row[2].as<string>(), row[3].as<int>()});
    }
    return books;
}

vector<string> parse_tags(const string& input) {
    vector<string> result;
    stringstream ss(input);
    string tag;
    while (getline(ss, tag, ',')) {
        size_t start = tag.find_first_not_of(" \t");
        size_t end = tag.find_last_not_of(" \t");
        if (start != string::npos && end != string::npos) {
            string trimmed = tag.substr(start, end - start + 1);
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
        
        pqxx::connection conn(db_url);
        ensure_tables(conn);
        
        string line;
        while (getline(cin, line)) {
            if (line.empty()) continue;
            
            istringstream iss(line);
            string cmd;
            iss >> cmd;
            
            if (cmd == "AddAuthor") {
                string name;
                getline(iss, name);
                size_t start = name.find_first_not_of(" \t");
                if (start != string::npos) name = name.substr(start);
                size_t end = name.find_last_not_of(" \t");
                if (end != string::npos) name = name.substr(0, end + 1);
                
                if (name.empty()) {
                    cout << "Failed to add author" << endl;
                    continue;
                }
                
                try {
                    pqxx::work w(conn);
                    w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)",
                                  generate_uuid(), name);
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to add author" << endl;
                }
                
            } else if (cmd == "ShowAuthors") {
                auto authors = get_authors(conn);
                for (size_t i = 0; i < authors.size(); ++i) {
                    cout << i + 1 << " " << authors[i].second << endl;
                }
                
            } else if (cmd == "DeleteAuthor") {
                string name;
                getline(iss, name);
                size_t start = name.find_first_not_of(" \t");
                if (start != string::npos) name = name.substr(start);
                size_t end = name.find_last_not_of(" \t");
                if (end != string::npos) name = name.substr(0, end + 1);
                
                if (name.empty()) {
                    auto authors = get_authors(conn);
                    if (authors.empty()) continue;
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i].second << endl;
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
                    name = authors[choice - 1].second;
                }
                
                try {
                    pqxx::work w(conn);
                    auto res = w.exec_params("DELETE FROM authors WHERE name = $1 RETURNING id", name);
                    if (res.empty()) {
                        cout << "Failed to delete author" << endl;
                    }
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to delete author" << endl;
                }
                
            } else if (cmd == "EditAuthor") {
                string name;
                getline(iss, name);
                size_t start = name.find_first_not_of(" \t");
                if (start != string::npos) name = name.substr(start);
                size_t end = name.find_last_not_of(" \t");
                if (end != string::npos) name = name.substr(0, end + 1);
                
                if (name.empty()) {
                    auto authors = get_authors(conn);
                    if (authors.empty()) continue;
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i].second << endl;
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
                    name = authors[choice - 1].second;
                }
                
                cout << "Enter new name:" << endl;
                string new_name;
                getline(cin, new_name);
                start = new_name.find_first_not_of(" \t");
                if (start != string::npos) new_name = new_name.substr(start);
                end = new_name.find_last_not_of(" \t");
                if (end != string::npos) new_name = new_name.substr(0, end + 1);
                
                try {
                    pqxx::work w(conn);
                    auto res = w.exec_params("UPDATE authors SET name = $1 WHERE name = $2 RETURNING id", 
                                              new_name, name);
                    if (res.empty()) {
                        cout << "Failed to edit author" << endl;
                    }
                    w.commit();
                } catch (const exception& e) {
                    cout << "Failed to edit author" << endl;
                }
                
            } else if (cmd == "ShowBooks") {
                auto books = get_books(conn);
                for (size_t i = 0; i < books.size(); ++i) {
                    cout << i + 1 << " " << get<1>(books[i]) << " by " 
                         << get<2>(books[i]) << ", " << get<3>(books[i]) << endl;
                }
                
            } else if (cmd == "AddBook") {
                int year;
                iss >> year;
                string title;
                getline(iss, title);
                size_t start = title.find_first_not_of(" \t");
                if (start != string::npos) title = title.substr(start);
                size_t end = title.find_last_not_of(" \t");
                if (end != string::npos) title = title.substr(0, end + 1);
                
                if (title.empty()) continue;
                
                cout << "Enter author name or empty line to select from list:" << endl;
                string author_input;
                getline(cin, author_input);
                start = author_input.find_first_not_of(" \t");
                if (start != string::npos) author_input = author_input.substr(start);
                end = author_input.find_last_not_of(" \t");
                if (end != string::npos) author_input = author_input.substr(0, end + 1);
                
                string author_id;
                
                if (!author_input.empty()) {
                    pqxx::read_transaction r(conn);
                    auto res = r.exec_params("SELECT id FROM authors WHERE name = $1", author_input);
                    if (!res.empty()) {
                        author_id = res[0][0].as<string>();
                    } else {
                        cout << "No author found. Do you want to add " << author_input << " (y/n)?" << endl;
                        string answer;
                        getline(cin, answer);
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
                    auto authors = get_authors(conn);
                    if (authors.empty()) continue;
                    cout << "Select author:" << endl;
                    for (size_t i = 0; i < authors.size(); ++i) {
                        cout << i + 1 << " " << authors[i].second << endl;
                    }
                    cout << "Enter author # or empty line to cancel" << endl;
                    string choice_line;
                    getline(cin, choice_line);
                    if (choice_line.empty()) continue;
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
