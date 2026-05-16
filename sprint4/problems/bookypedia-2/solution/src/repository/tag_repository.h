#pragma once
#include "domain/tag.h"
#include "domain/book.h"
#include <memory>
#include <vector>

namespace repository {

class TagRepository {
public:
    virtual ~TagRepository() = default;
    
    virtual void Save(const domain::Tag& tag) = 0;
    virtual std::vector<std::string> FindByBook(const domain::Book::Id& book_id) const = 0;
    virtual void DeleteByBook(const domain::Book::Id& book_id) = 0;
    virtual void DeleteByAuthor(const domain::Author::Id& author_id) = 0;
};

} // namespace repository
