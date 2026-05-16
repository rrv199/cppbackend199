#include "tag_repository.h"
#include <pqxx/pqxx>
#include "util/tagged_uuid.h"
#include <set>

namespace repository {

class TagRepositoryImpl : public TagRepository {
public:
    explicit TagRepositoryImpl(std::shared_ptr<pqxx::connection> conn) : conn_(std::move(conn)) {}
    
    void Save(const domain::Tag& tag) override {
        pqxx::work w(*conn_);
        w.exec_params(
            "INSERT INTO book_tags (book_id, tag) VALUES ($1, $2) ON CONFLICT DO NOTHING",
            util::ToString(tag.GetBookId()), tag.GetTag());
        w.commit();
    }
    
    std::vector<std::string> FindByBook(const domain::Book::Id& book_id) const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec_params(
            "SELECT tag FROM book_tags WHERE book_id = $1 ORDER BY tag",
            util::ToString(book_id));
        std::vector<std::string> tags;
        for (const auto& row : res) {
            tags.push_back(row[0].as<std::string>());
        }
        return tags;
    }
    
    void DeleteByBook(const domain::Book::Id& book_id) override {
        pqxx::work w(*conn_);
        w.exec_params("DELETE FROM book_tags WHERE book_id = $1", util::ToString(book_id));
        w.commit();
    }
    
    void DeleteByAuthor(const domain::Author::Id& author_id) override {
        pqxx::work w(*conn_);
        w.exec_params(
            "DELETE FROM book_tags WHERE book_id IN (SELECT id FROM books WHERE author_id = $1)",
            util::ToString(author_id));
        w.commit();
    }
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace repository
