#pragma once
#include "use_cases.h"
#include "repository/author_repository.h"
#include "repository/book_repository.h"
#include <memory>

namespace app {

class UseCasesImpl : public UseCases {
public:
    UseCasesImpl(std::unique_ptr<repository::AuthorRepository> authors,
                 std::unique_ptr<repository::BookRepository> books);
    
    void AddAuthor(const std::string& name) override;
    std::vector<domain::Author> ShowAuthors() override;
    void AddBook(int year, const std::string& title, const domain::Author::Id& author_id) override;
    std::vector<domain::Author> GetAuthorsForSelection() override;
    std::vector<domain::Book> ShowAuthorBooks(const domain::Author::Id& author_id) override;
    std::vector<domain::Book> ShowBooks() override;
    
private:
    std::unique_ptr<repository::AuthorRepository> authors_;
    std::unique_ptr<repository::BookRepository> books_;
};

} // namespace app
