#pragma once
#include "util/tagged_uuid.h"
#include <string>

namespace domain {

class Author {
public:
    using Id = util::AuthorId;
    
    Author(Id id, std::string name) : id_(std::move(id)), name_(std::move(name)) {}
    
    const Id& GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    
private:
    Id id_;
    std::string name_;
};

} // namespace domain
