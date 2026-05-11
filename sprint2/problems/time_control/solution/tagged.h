#pragma once

namespace tagged {

template<typename T, typename Tag>
class Tagged {
public:
    Tagged() = default;
    explicit Tagged(T value) : value_(std::move(value)) {}
    
    const T& operator*() const { return value_; }
    const T* operator->() const { return &value_; }
    
    bool operator==(const Tagged& other) const { return value_ == other.value_; }
    bool operator!=(const Tagged& other) const { return value_ != other.value_; }
    
private:
    T value_;
};

} // namespace tagged
