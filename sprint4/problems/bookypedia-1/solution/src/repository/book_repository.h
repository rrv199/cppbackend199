#pragma once
#include "domain/book.h"
#include "domain/author.h"
#include <memory>
#include <vector>
#include <optional>

namespace repository {

class BookRepository {
public:
    virtual ~BookRepository() = default;
    
    virtual void Save(const domain::Book& book) = 0;
    virtual std::vector<domain::Book> FindByAuthor(const domain::Author::Id& author_id) const = 0;
    virtual std::vector<domain::Book> GetAllSortedByTitle() const = 0;
};

} // namespace repository
