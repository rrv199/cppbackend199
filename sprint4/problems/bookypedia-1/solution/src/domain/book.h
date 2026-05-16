#pragma once
#include "util/tagged_uuid.h"
#include "author.h"
#include <string>

namespace domain {

class Book {
public:
    using Id = util::BookId;
    
    Book(Id id, Author::Id author_id, std::string title, int publication_year)
        : id_(std::move(id)), author_id_(std::move(author_id)), 
          title_(std::move(title)), publication_year_(publication_year) {}
    
    const Id& GetId() const { return id_; }
    const Author::Id& GetAuthorId() const { return author_id_; }
    const std::string& GetTitle() const { return title_; }
    int GetPublicationYear() const { return publication_year_; }
    
private:
    Id id_;
    Author::Id author_id_;
    std::string title_;
    int publication_year_;
};

} // namespace domain
