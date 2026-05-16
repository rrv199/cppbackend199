#include <iostream>
#include <string>
using namespace std;

int main() {
    int year = 1906;
    string title = "White Fang";
    string book_id = "test-id";
    string author_id = "test-author";
    
    cout << "INSERT INTO books (id, author_id, title, publication_year) VALUES (" 
         << book_id << ", " << author_id << ", " << title << ", " << year << ")" << endl;
    
    // Правильный порядок
    cout << "Correct: VALUES ($1, $2, $3, $4) -> " 
         << book_id << ", " << author_id << ", " << title << ", " << year << endl;
    
    // Неправильный порядок (который дает 6 White Fang)
    cout << "Wrong: VALUES ($1, $2, $3, $4) -> " 
         << book_id << ", " << author_id << ", " << year << ", " << title << endl;
    
    return 0;
}
