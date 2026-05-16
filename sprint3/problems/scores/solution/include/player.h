#pragma once
#include <vector>
#include <cstddef>

struct BagItem {
    size_t id;
    size_t type;
    size_t value;  // стоимость предмета
};

class Player {
public:
    Player(size_t bag_capacity) : bag_capacity_(bag_capacity), score_(0) {}
    
    bool AddItem(size_t id, size_t type, size_t value) {
        if (bag_.size() >= bag_capacity_) {
            return false;
        }
        bag_.push_back({id, type, value});
        return true;
    }
    
    size_t DeliverItems() {
        size_t total_value = 0;
        for (const auto& item : bag_) {
            total_value += item.value;
        }
        score_ += total_value;
        bag_.clear();
        return total_value;
    }
    
    const std::vector<BagItem>& GetBag() const { return bag_; }
    size_t GetBagSize() const { return bag_.size(); }
    size_t GetBagCapacity() const { return bag_capacity_; }
    size_t GetScore() const { return score_; }
    
private:
    size_t bag_capacity_;
    size_t score_;
    std::vector<BagItem> bag_;
};
