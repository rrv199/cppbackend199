#pragma once
#include "domain/author.h"
#include <memory>
#include <optional>
#include <vector>

namespace repository {

class AuthorRepository {
public:
    virtual ~AuthorRepository() = default;
    
    virtual void Save(const domain::Author& author) = 0;
    virtual std::optional<domain::Author> FindById(const domain::Author::Id& id) const = 0;
    virtual std::optional<domain::Author> FindByName(const std::string& name) const = 0;
    virtual std::vector<domain::Author> GetAllSortedByName() const = 0;
};

} // namespace repository
