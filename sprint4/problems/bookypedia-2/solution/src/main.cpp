#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <pqxx/pqxx>

using namespace std;

void EnsureTablesExist(pqxx::connection& conn) {
    pqxx::work w(conn);
    w.exec(
        "CREATE EXTENSION IF NOT EXISTS pgcrypto;"
        "CREATE TABLE IF NOT EXISTS authors ("
        "id uuid PRIMARY KEY, "
        "name varchar(100) UNIQUE NOT NULL);"
        "CREATE TABLE IF NOT EXISTS books ("
        "id uuid PRIMARY KEY, "
        "author_id uuid NOT NULL REFERENCES authors(id) ON DELETE CASCADE, "
        "title varchar(100) NOT NULL, "
        "publication_year integer NOT NULL);"
        "CREATE TABLE IF NOT EXISTS book_tags ("
        "book_id uuid NOT NULL REFERENCES books(id) ON DELETE CASCADE, "
        "tag varchar(30) NOT NULL, "
        "PRIMARY KEY (book_id, tag));");
    w.commit();
}

int main() {
    try {
        const char* db_url = getenv("BOOKYPEDIA_DB_URL");
        if (!db_url) {
            cerr << "BOOKYPEDIA_DB_URL environment variable not set" << endl;
            return 1;
        }
        
        pqxx::connection conn(db_url);
        EnsureTablesExist(conn);
        
        string line;
        while (getline(cin, line)) {
            if (line.empty()) continue;
            cout << line << endl;
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
