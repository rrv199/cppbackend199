#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
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
    if (input.empty()) return {};
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
            w.exec("CREATE TABLE IF NOT EXISTS authors (id uuid PRIMARY KEY, name varchar(100) UNIQUE NOT NULL);");
            w.exec("CREATE TABLE IF NOT EXISTS books (id uuid PRIMARY KEY, author_id uuid NOT NULL REFERENCES authors(id) ON DELETE CASCADE, title varchar(100) NOT NULL, publication_year integer NOT NULL);");
            w.exec("CREATE TABLE IF NOT EXISTS book_tags (book_id uuid NOT NULL REFERENCES books(id) ON DELETE CASCADE, tag varchar(30) NOT NULL, PRIMARY KEY (book_id, tag));");
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
                    w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)", generate_uuid(), name);
                    w.commit();
                } catch (const exception& e) {
                    string err = e.what();
                    if (err.find("duplicate") == string::npos) {
                        cout << "Failed to add author" << endl;
                    }
                }

            } else if (cmd == "ShowAuthors") {
                try {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);
                    auto res = t.exec("SELECT name FROM authors ORDER BY name");
                    int i = 1;
                    for (const auto& row : res) {
                        cout << i++ << " " << row[0].as<string>() << endl;
                    }
                } catch (...) {}

            } else if (cmd == "ShowBooks") {
                try {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);
                    auto res = t.exec("SELECT b.title, a.name, b.publication_year FROM books b JOIN authors a ON b.author_id = a.id ORDER BY b.title, a.name, b.publication_year");
                    int i = 1;
                    for (const auto& row : res) {
                        cout << i++ << " " << row[0].as<string>() << " by " << row[1].as<string>() << ", " << row[2].as<int>() << endl;
                    }
                } catch (...) {}

            } else if (cmd == "ShowBook") {
                string title = trim(line.substr(cmd.length()));
                if (title.empty()) continue;

                try {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);

                    auto res = t.exec_params(
                        "SELECT b.id, b.title, a.name, b.publication_year "
                        "FROM books b JOIN authors a ON b.author_id = a.id "
                        "WHERE b.title = $1", title.c_str());

                    if (res.empty()) {
                        continue;
                    }

                    const auto& row = res[0];
                    cout << "Title: " << row[1].as<string>() << endl;
                    cout << "Author: " << row[2].as<string>() << endl;
                    cout << "Publication year: " << row[3].as<int>() << endl;

                    auto tag_res = t.exec_params("SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag", row[0].as<string>());
                    if (!tag_res.empty()) {
                        cout << "Tags: ";
                        for (size_t j = 0; j < tag_res.size(); ++j) {
                            if (j > 0) cout << ", ";
                            cout << tag_res[j][0].as<string>();
                        }
                        cout << endl;
                    }
                } catch (const exception& e) {
                    cerr << "ShowBook error: " << e.what() << endl;
                }

            } else if (cmd == "DeleteBook") {
                string title = trim(line.substr(cmd.length()));
                if (title.empty()) continue;

                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);

                    auto res = w.exec_params("SELECT id FROM books WHERE title = $1", title.c_str());
                    if (res.empty()) {
                        cout << "Failed to delete book" << endl;
                        continue;
                    }

                    w.exec_params("DELETE FROM books WHERE title = $1", title.c_str());
                    w.commit();
                } catch (const exception& e) {
                    cerr << "DeleteBook error: " << e.what() << endl;
                    cout << "Failed to delete book" << endl;
                }

            } else if (cmd == "EditBook") {
                string title = trim(line.substr(cmd.length()));
                if (title.empty()) continue;

                string book_id;
                string current_title;
                int current_year;
                vector<string> existing_tags;
                
                // Чтение - используем отдельное соединение
                {
                    pqxx::connection conn_read(db_url);
                    pqxx::nontransaction t(conn_read);
                    
                    auto res = t.exec_params("SELECT id, title, publication_year FROM books WHERE title = $1", title.c_str());
                    if (res.empty()) {
                        cout << "Book not found" << endl;
                        continue;
                    }
                    
                    book_id = res[0][0].as<string>();
                    current_title = res[0][1].as<string>();
                    current_year = res[0][2].as<int>();
                    
                    auto tag_res = t.exec_params("SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag", book_id);
                    for (const auto& row : tag_res) {
                        existing_tags.push_back(row[0].as<string>());
                    }
                }
                
                string current_tags;
                for (size_t i = 0; i < existing_tags.size(); ++i) {
                    if (i > 0) current_tags += ", ";
                    current_tags += existing_tags[i];
                }
                
                cout << "Enter new title or empty line to use the current one (" << current_title << "):" << endl;
                string new_title;
                getline(cin, new_title);
                new_title = trim(new_title);
                if (new_title.empty()) new_title = current_title;
                
                cout << "Enter publication year or empty line to use the current one (" << current_year << "):" << endl;
                string year_str;
                getline(cin, year_str);
                int new_year = current_year;
                if (!year_str.empty()) new_year = stoi(year_str);
                
                cout << "Enter tags (current tags: " << (current_tags.empty() ? "none" : current_tags) << "):" << endl;
                string tags_line;
                getline(cin, tags_line);
                
                vector<string> new_tags;
                if (tags_line.empty()) {
                    new_tags = existing_tags;
                } else {
                    new_tags = parse_tags(tags_line);
                }
                
                // Запись - отдельное соединение
                {
                    pqxx::connection conn_write(db_url);
                    pqxx::work w(conn_write);
                    w.exec_params("UPDATE books SET title = $1, publication_year = $2 WHERE id = $3",
                                  new_title, new_year, book_id);
                    w.exec_params("DELETE FROM book_tags WHERE book_id = $1", book_id);
                    for (const auto& tag : new_tags) {
                        if (!tag.empty()) {
                            w.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)", book_id, tag);
                        }
                    }
                    w.commit();
                }

            } else if (cmd == "AddBook") {
                int year;
                iss >> year;
                string remaining;
                getline(iss, remaining);
                string title = trim(remaining);
                if (title.empty()) continue;

                // Проверка на дубликат
                {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);
                    auto check = t.exec_params("SELECT id FROM books WHERE title = $1 AND publication_year = $2", title.c_str(), year);
                    if (!check.empty()) {
                        continue;
                    }
                }

                cout << "Enter author name or empty line to select from list:" << endl;
                string author_input;
                getline(cin, author_input);
                author_input = trim(author_input);
                string author_id;

                if (!author_input.empty()) {
                    try {
                        pqxx::connection conn(db_url);
                        pqxx::nontransaction t(conn);
                        auto res = t.exec_params("SELECT id FROM authors WHERE name = $1", author_input.c_str());
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
                            w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)", author_id, author_input);
                            w.commit();
                        }
                    } catch (...) {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                } else {
                    try {
                        pqxx::connection conn(db_url);
                        pqxx::nontransaction t(conn);
                        auto res = t.exec("SELECT id, name FROM authors ORDER BY name");
                        vector<pair<string,string>> authors;
                        for (const auto& row : res) authors.push_back({row[0].as<string>(), row[1].as<string>()});
                        if (authors.empty()) {
                            cout << "Failed to add book" << endl;
                            continue;
                        }
                        cout << "Select author:" << endl;
                        for (size_t i = 0; i < authors.size(); i++) cout << i+1 << " " << authors[i].second << endl;
                        cout << "Enter author # or empty line to cancel" << endl;
                        string choice;
                        getline(cin, choice);
                        if (choice.empty()) {
                            cout << "Failed to add book" << endl;
                            continue;
                        }
                        int idx = stoi(choice);
                        if (idx < 1 || idx > (int)authors.size()) {
                            cout << "Failed to add book" << endl;
                            continue;
                        }
                        author_id = authors[idx-1].first;
                    } catch (...) {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                }

                cout << "Enter tags (comma separated):" << endl;
                string tags_line;
                getline(cin, tags_line);
                auto tags = parse_tags(tags_line);

                string book_id = generate_uuid();
                try {
                    pqxx::connection conn(db_url);
                    pqxx::work w(conn);
                    w.exec_params("INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)",
                                  book_id, author_id, title, year);
                    for (const auto& tag : tags) {
                        w.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)", book_id, tag);
                    }
                    w.commit();
                } catch (const exception& e) {
                    cerr << "DB Error: " << e.what() << endl;
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
