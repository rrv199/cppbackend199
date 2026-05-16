// ... existing code ...

        // Initialize tables
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
        }
        
// ... rest of code ...
