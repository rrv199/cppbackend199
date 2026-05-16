// Добавьте этот блок после ShowBook и перед AddBook
            } else if (cmd == "DeleteBook") {
                string title = trim(line.substr(cmd.length()));
                
                try {
                    pqxx::connection conn(db_url);
                    pqxx::nontransaction t(conn);
                    
                    if (!title.empty()) {
                        auto res = t.exec_params(
                            "SELECT id, title, a.name, publication_year "
                            "FROM books b JOIN authors a ON b.author_id = a.id "
                            "WHERE title = $1", title.c_str());
                        
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
                        } else {
                            cout << "Select book:" << endl;
                            vector<string> book_ids;
                            for (size_t i = 0; i < res.size(); ++i) {
                                const auto& row = res[i];
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
                            
                            pqxx::work w(conn);
                            w.exec_params("DELETE FROM books WHERE id = $1", book_ids[choice - 1]);
                            w.commit();
                            continue;
                        }
                    }
                    
                    // No title provided - show all books
                    auto all_books = t.exec(
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
                    
                    pqxx::work w(conn);
                    w.exec_params("DELETE FROM books WHERE id = $1", book_ids[choice - 1]);
                    w.commit();
                } catch (...) {
                    cout << "Failed to delete book" << endl;
                }
                
