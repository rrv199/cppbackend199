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
    vector<string> result;
    stringstream ss(input);
    string tag;
    while (getline(ss, tag, ',')) {
        string t = trim(tag);
        if (!t.empty()) result.push_back(t);
    }
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}

int main() {
    const char* db_url = getenv("BOOKYPEDIA_DB_URL");
    if (!db_url) {
        cerr << "BOOKYPEDIA_DB_URL environment variable not set" << endl;
        return 1;
    }
    
    // Создаем таблицы
    {
        pqxx::connection conn(db_url);
        pqxx::work w(conn);
        w.exec("CREATE EXTENSION IF NOT EXISTS pgcrypto;");
        w.exec("DROP TABLE IF EXISTS book_tags CASCADE;");
        w.exec("DROP TABLE IF EXISTS books CASCADE;");
        w.exec("DROP TABLE IF EXISTS authors CASCADE;");
        w.exec("CREATE TABLE authors (id uuid PRIMARY KEY, name varchar(100) UNIQUE NOT NULL);");
        w.exec("CREATE TABLE books (id uuid PRIMARY KEY, author_id uuid NOT NULL REFERENCES authors(id) ON DELETE CASCADE, title varchar(100) NOT NULL, publication_year integer NOT NULL);");
        w.exec("CREATE TABLE book_tags (book_id uuid NOT NULL REFERENCES books(id) ON DELETE CASCADE, tag varchar(30) NOT NULL, PRIMARY KEY (book_id, tag));");
        w.commit();
                    system("sync");
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
                    system("sync");
            } catch (...) {
                cout << "Failed to add author" << endl;
            }
        }
        else if (cmd == "ShowAuthors") {
            pqxx::connection conn(db_url);
            pqxx::read_transaction t(conn);
            auto res = t.exec("SELECT name FROM authors ORDER BY name");
            int i = 1;
            for (const auto& row : res) {
                cout << i++ << " " << row[0].as<string>() << endl;
            }
        }
        else if (cmd == "ShowBooks") {
            pqxx::connection conn(db_url);
            pqxx::read_transaction t(conn);
            auto res = t.exec("SELECT b.title, a.name, b.publication_year FROM books b JOIN authors a ON b.author_id = a.id ORDER BY b.title");
            int i = 1;
            for (const auto& row : res) {
                cout << i++ << " " << row[0].as<string>() << " by " << row[1].as<string>() << ", " << row[2].as<int>() << endl;
            }
        }
        else if (cmd == "AddBook") {
            int year;
            iss >> year;
            string title = trim(line.substr(cmd.length() + to_string(year).length() + 1));
            if (title.empty()) continue;
            
            cout << "Enter author name or empty line to select from list:" << endl;
            string author_name;
            getline(cin, author_name);
            author_name = trim(author_name);
            
            string author_id;
            {
                pqxx::connection conn(db_url);
                pqxx::read_transaction t(conn);
                auto res = t.exec_params("SELECT id FROM authors WHERE name = $1", author_name.c_str());
                if (!res.empty()) {
                    author_id = res[0][0].as<string>();
                } else {
                    cout << "No author found. Do you want to add " << author_name << " (y/n)?" << endl;
                    string answer;
                    getline(cin, answer);
                    if (answer != "y" && answer != "Y") {
                        cout << "Failed to add book" << endl;
                        continue;
                    }
                    author_id = generate_uuid();
                    pqxx::work w(conn);
                    w.exec_params("INSERT INTO authors (id, name) VALUES ($1, $2)", author_id, author_name);
                    w.commit();
                    system("sync");
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
                    system("sync");
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
                cout << "Failed to add book" << endl;
            }
        }
    }
    return 0;
}
