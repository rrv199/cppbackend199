#pragma once
#include "domain/author.h"
#include "domain/book.h"
#include <string>
#include <vector>
#include <optional>

namespace app {

class UseCases {
public:
    virtual ~UseCases() = default;
    
    virtual void AddAuthor(const std::string& name) = 0;
    virtual std::vector<domain::Author> ShowAuthors() = 0;
    virtual void AddBook(int year, const std::string& title, const domain::Author::Id& author_id) = 0;
    virtual std::vector<domain::Author> GetAuthorsForSelection() = 0;
    virtual std::vector<domain::Book> ShowAuthorBooks(const domain::Author::Id& author_id) = 0;
    virtual std::vector<domain::Book> ShowBooks() = 0;
};

} // namespace app
