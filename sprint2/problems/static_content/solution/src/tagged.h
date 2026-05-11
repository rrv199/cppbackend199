#pragma once

#include <string>
#include <utility>

namespace util {

template <typename Value, typename Tag>
class Tagged {
public:
    explicit Tagged(const Value& v) : value_(v) {}
    explicit Tagged(Value&& v) : value_(std::move(v)) {}

    const Value& operator*() const { return value_; }
    Value& operator*() { return value_; }

    bool operator==(const Tagged& other) const { return value_ == other.value_; }

private:
    Value value_;
};

}  // namespace util
