#include "author_repository.h"
#include <pqxx/pqxx>
#include "util/tagged_uuid.h"

namespace repository {

class AuthorRepositoryImpl : public AuthorRepository {
public:
    explicit AuthorRepositoryImpl(std::shared_ptr<pqxx::connection> conn) : conn_(std::move(conn)) {}
    
    void Save(const domain::Author& author) override {
        pqxx::work w(*conn_);
        w.exec_params(
            "INSERT INTO authors (id, name) VALUES ($1, $2) ON CONFLICT (name) DO NOTHING",
            util::ToString(author.GetId()), author.GetName());
        w.commit();
    }
    
    std::optional<domain::Author> FindById(const domain::Author::Id& id) const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec_params("SELECT id, name FROM authors WHERE id = $1", util::ToString(id));
        if (res.empty()) {
            return std::nullopt;
        }
        const auto& row = res[0];
        auto uuid = util::UUIDFromString(row[0].as<std::string>());
        return domain::Author(domain::Author::Id(uuid), row[1].as<std::string>());
    }
    
    std::optional<domain::Author> FindByName(const std::string& name) const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec_params("SELECT id, name FROM authors WHERE name = $1", name);
        if (res.empty()) {
            return std::nullopt;
        }
        const auto& row = res[0];
        auto uuid = util::UUIDFromString(row[0].as<std::string>());
        return domain::Author(domain::Author::Id(uuid), row[1].as<std::string>());
    }
    
    std::vector<domain::Author> GetAllSortedByName() const override {
        pqxx::read_transaction r(*conn_);
        auto res = r.exec("SELECT id, name FROM authors ORDER BY name");
        std::vector<domain::Author> authors;
        for (const auto& row : res) {
            auto uuid = util::UUIDFromString(row[0].as<std::string>());
            authors.emplace_back(domain::Author::Id(uuid), row[1].as<std::string>());
        }
        return authors;
    }
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};

} // namespace repository
