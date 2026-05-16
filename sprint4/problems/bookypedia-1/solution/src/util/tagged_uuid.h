#pragma once
#include "tagged.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

namespace util {

using UUID = boost::uuids::uuid;

namespace detail {
    struct AuthorIdTag {};
    struct BookIdTag {};
} // namespace detail

using AuthorId = Tagged<UUID, detail::AuthorIdTag>;
using BookId = Tagged<UUID, detail::BookIdTag>;

inline std::string ToString(const AuthorId& id) {
    return boost::uuids::to_string(*id);
}

inline std::string ToString(const BookId& id) {
    return boost::uuids::to_string(*id);
}

inline UUID GenerateUUID() {
    static boost::uuids::random_generator gen;
    return gen();
}

inline UUID UUIDFromString(const std::string& str) {
    boost::uuids::string_generator gen;
    return gen(str);
}

} // namespace util
