#include "book_repository.h"
#include <pqxx/pqxx>
#include "util/tagged_uuid.h"

namespace repository {

class BookRepositoryImpl : public BookRepository {
public:
    explicit BookRepositoryImpl(std::shared_ptr<pqxx::connection> conn) : conn_(std::move(conn)) {}
    
    void Save(const domain::Book& book) override {
        pqxx::work w(*conn_);
        w.exec_params(
            "INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)",
            util::ToString(book.GetId()), util::ToString(book.GetAuthorId()),
            book.GetTitle(), book.GetPublicationYear());
        w.commit();
    }
    
    std::vector<domain::Book> FindByAuthor(const domain::Author::Id& author_id) const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec_params(
            "SELECT id, title, publication_year FROM books WHERE author_id = $1 "
            "ORDER BY publication_year, title",
            util::ToString(author_id));
        
        std::vector<domain::Book> books;
        for (const auto& row : res) {
            auto uuid = util::UUIDFromString(row[0].as<std::string>());
            books.emplace_back(
                domain::Book::Id(uuid), author_id,
                row[1].as<std::string>(), row[2].as<int>());
        }
        return books;
    }
    
    std::vector<domain::Book> GetAllSortedByTitle() const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec(
            "SELECT id, author_id, title, publication_year FROM books ORDER BY title");
        
        std::vector<domain::Book> books;
        for (const auto& row : res) {
            auto id_uuid = util::UUIDFromString(row[0].as<std::string>());
            auto author_uuid = util::UUIDFromString(row[1].as<std::string>());
            books.emplace_back(
                domain::Book::Id(id_uuid), domain::Author::Id(author_uuid),
                row[2].as<std::string>(), row[3].as<int>());
        }
        return books;
    }
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace repository
