#include "use_cases_impl.h"
#include "util/tagged_uuid.h"
#include <stdexcept>

namespace app {

UseCasesImpl::UseCasesImpl(std::unique_ptr<repository::AuthorRepository> authors,
                           std::unique_ptr<repository::BookRepository> books)
    : authors_(std::move(authors)), books_(std::move(books)) {}

void UseCasesImpl::AddAuthor(const std::string& name) {
    if (name.empty()) {
        throw std::runtime_error("Empty author name");
    }
    
    // Проверяем, существует ли уже такой автор
    if (authors_->FindByName(name).has_value()) {
        throw std::runtime_error("Author already exists");
    }
    
    auto id = util::AuthorId(util::GenerateUUID());
    domain::Author author(id, name);
    authors_->Save(author);
}

std::vector<domain::Author> UseCasesImpl::ShowAuthors() {
    return authors_->GetAllSortedByName();
}

void UseCasesImpl::AddBook(int year, const std::string& title, const domain::Author::Id& author_id) {
    auto id = util::BookId(util::GenerateUUID());
    domain::Book book(id, author_id, title, year);
    books_->Save(book);
}

std::vector<domain::Author> UseCasesImpl::GetAuthorsForSelection() {
    return authors_->GetAllSortedByName();
}

std::vector<domain::Book> UseCasesImpl::ShowAuthorBooks(const domain::Author::Id& author_id) {
    return books_->FindByAuthor(author_id);
}

std::vector<domain::Book> UseCasesImpl::ShowBooks() {
    return books_->GetAllSortedByTitle();
}

} // namespace app
