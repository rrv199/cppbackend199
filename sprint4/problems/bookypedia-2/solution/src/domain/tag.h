#pragma once
#include "util/tagged_uuid.h"
#include "book.h"
#include <string>

namespace domain {

class Tag {
public:
    Tag(Book::Id book_id, std::string tag)
        : book_id_(std::move(book_id)), tag_(std::move(tag)) {}
    
    const Book::Id& GetBookId() const { return book_id_; }
    const std::string& GetTag() const { return tag_; }
    
private:
    Book::Id book_id_;
    std::string tag_;
};

} // namespace domain
