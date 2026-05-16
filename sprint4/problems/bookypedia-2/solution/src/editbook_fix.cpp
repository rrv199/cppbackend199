// Замените блок EditBook в main.cpp на этот:

            } else if (cmd == "EditBook") {
                string title = trim(line.substr(cmd.length()));
                
                try {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);
                    
                    // Find books matching the title
                    auto res = t.exec_params(
                        "SELECT b.id, b.title FROM books b WHERE b.title = $1", title.c_str());
                    
                    // If no books found and title was provided
                    if (res.empty() && !title.empty()) {
                        cout << "Book not found" << endl;
                        continue;
                    }
                    
                    string book_id;
                    string current_title;
                    
                    // If exactly one book found and title was provided
                    if (res.size() == 1 && !title.empty()) {
                        book_id = res[0][0].as<string>();
                        current_title = res[0][1].as<string>();
                    } else {
                        // Show all books for selection
                        auto all_books = t.exec(
                            "SELECT id, title FROM books ORDER BY title");
                        
                        if (all_books.empty()) {
                            cout << "Book not found" << endl;
                            continue;
                        }
                        
                        cout << "Select book:" << endl;
                        vector<pair<string, string>> select_books;
                        for (size_t i = 0; i < all_books.size(); ++i) {
                            const auto& row = all_books[i];
                            auto author_res = t.exec_params(
                                "SELECT a.name FROM authors a JOIN books b ON b.author_id = a.id WHERE b.id = $1", row[0].as<string>());
                            string author_name = author_res.empty() ? "" : author_res[0][0].as<string>();
                            cout << i + 1 << " " << row[1].as<string>() << " by " << author_name << endl;
                            select_books.push_back({row[0].as<string>(), row[1].as<string>()});
                        }
                        cout << "Enter the book # or empty line to cancel:" << endl;
                        string choice_line;
                        getline(cin, choice_line);
                        if (choice_line.empty()) continue;
                        int choice = stoi(choice_line);
                        if (choice < 1 || choice > (int)select_books.size()) {
                            cout << "Book not found" << endl;
                            continue;
                        }
                        
                        book_id = select_books[choice - 1].first;
                        current_title = select_books[choice - 1].second;
                    }
                    
                    cout << "Enter new title or empty line to use the current one (" << current_title << "):" << endl;
                    string new_title;
                    getline(cin, new_title);
                    new_title = trim(new_title);
                    if (new_title.empty()) new_title = current_title;
                    
                    auto year_res = t.exec_params("SELECT publication_year FROM books WHERE id = $1", book_id);
                    int current_year = year_res[0][0].as<int>();
                    cout << "Enter publication year or empty line to use the current one (" << current_year << "):" << endl;
                    string year_str;
                    getline(cin, year_str);
                    year_str = trim(year_str);
                    int new_year = current_year;
                    if (!year_str.empty()) new_year = stoi(year_str);
                    
                    auto tag_res = t.exec_params("SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag", book_id);
                    string tags_str;
                    for (size_t i = 0; i < tag_res.size(); ++i) {
                        if (i > 0) tags_str += ", ";
                        tags_str += tag_res[i][0].as<string>();
                    }
                    cout << "Enter tags (current tags: " << (tags_str.empty() ? "none" : tags_str) << "):" << endl;
                    string tags_line;
                    getline(cin, tags_line);
                    tags_line = trim(tags_line);
                    
                    vector<string> new_tags;
                    if (tags_line.empty()) {
                        // Keep existing tags
                        for (const auto& row : tag_res) {
                            new_tags.push_back(row[0].as<string>());
                        }
                    } else {
                        new_tags = parse_tags(tags_line);
                    }
                    
                    pqxx::work w(conn);
                    w.exec_params("UPDATE books SET title = $1, publication_year = $2 WHERE id = $3", 
                                  new_title, new_year, book_id);
                    w.exec_params("DELETE FROM book_tags WHERE book_id = $1", book_id);
                    for (const auto& tag : new_tags) {
                        if (!tag.empty()) {
                            w.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2)", book_id, tag);
                        }
                    }
                    w.commit();
                } catch (const exception& e) {
                    cerr << "EditBook error: " << e.what() << endl;
                    cout << "Book not found" << endl;
                }
                
