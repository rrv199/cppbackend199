#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <pqxx/pqxx>
#include "app/use_cases_impl.h"
#include "repository/author_repository.cpp"
#include "repository/book_repository.cpp"

using namespace std;

void EnsureTablesExist(pqxx::connection& conn) {
    pqxx::work w(conn);
    w.exec(
        "CREATE TABLE IF NOT EXISTS authors ("
        "id uuid PRIMARY KEY, "
        "name varchar(100) UNIQUE NOT NULL)");
    
    w.exec(
        "CREATE TABLE IF NOT EXISTS books ("
        "id uuid PRIMARY KEY, "
        "author_id uuid NOT NULL REFERENCES authors(id) ON DELETE CASCADE, "
        "title varchar(100) NOT NULL, "
        "publication_year integer NOT NULL)");
    w.commit();
}

void PrintAuthors(const vector<domain::Author>& authors) {
    for (size_t i = 0; i < authors.size(); ++i) {
        cout << i + 1 << " " << authors[i].GetName() << endl;  // Без точки
    }
}

void PrintBooks(const vector<domain::Book>& books) {
    for (size_t i = 0; i < books.size(); ++i) {
        cout << i + 1 << " " << books[i].GetTitle() << ", " << books[i].GetPublicationYear() << endl;  // Без точки
    }
}

int main() {
    try {
        const char* db_url = getenv("BOOKYPEDIA_DB_URL");
        if (!db_url) {
            cerr << "BOOKYPEDIA_DB_URL environment variable not set" << endl;
            return 1;
        }
        
        auto conn = make_shared<pqxx::connection>(db_url);
        EnsureTablesExist(*conn);
        
        auto author_repo = make_unique<repository::AuthorRepositoryImpl>(conn);
        auto book_repo = make_unique<repository::BookRepositoryImpl>(conn);
        app::UseCasesImpl use_cases(move(author_repo), move(book_repo));
        
        string line;
        while (getline(cin, line)) {
            if (line.empty()) continue;
            
            istringstream iss(line);
            string command;
            iss >> command;
            
            if (command == "AddAuthor") {
                string name;
                getline(iss, name);
                size_t start = name.find_first_not_of(" \t");
                if (start != string::npos) {
                    name = name.substr(start);
                }
                size_t end = name.find_last_not_of(" \t");
                if (end != string::npos) {
                    name = name.substr(0, end + 1);
                }
                
                try {
                    use_cases.AddAuthor(name);
                } catch (const exception& e) {
                    cout << "Failed to add author" << endl;
                }
                
            } else if (command == "ShowAuthors") {
                auto authors = use_cases.ShowAuthors();
                PrintAuthors(authors);
                
            } else if (command == "AddBook") {
                int year;
                iss >> year;
                string title;
                getline(iss, title);
                size_t start = title.find_first_not_of(" \t");
                if (start != string::npos) {
                    title = title.substr(start);
                }
                size_t end = title.find_last_not_of(" \t");
                if (end != string::npos) {
                    title = title.substr(0, end + 1);
                }
                
                auto authors = use_cases.GetAuthorsForSelection();
                if (authors.empty()) continue;
                
                cout << "Select author:" << endl;
                PrintAuthors(authors);
                cout << "Enter author # or empty line to cancel" << endl;
                
                string choice_line;
                getline(cin, choice_line);
                if (choice_line.empty()) continue;
                
                int choice = stoi(choice_line);
                if (choice >= 1 && choice <= static_cast<int>(authors.size())) {
                    use_cases.AddBook(year, title, authors[choice - 1].GetId());
                }
                
            } else if (command == "ShowAuthorBooks") {
                auto authors = use_cases.GetAuthorsForSelection();
                if (authors.empty()) continue;
                
                cout << "Select author:" << endl;
                PrintAuthors(authors);
                cout << "Enter author # or empty line to cancel" << endl;
                
                string choice_line;
                getline(cin, choice_line);
                if (choice_line.empty()) continue;
                
                int choice = stoi(choice_line);
                if (choice >= 1 && choice <= static_cast<int>(authors.size())) {
                    auto books = use_cases.ShowAuthorBooks(authors[choice - 1].GetId());
                    PrintBooks(books);
                }
                
            } else if (command == "ShowBooks") {
                auto books = use_cases.ShowBooks();
                PrintBooks(books);
            }
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
