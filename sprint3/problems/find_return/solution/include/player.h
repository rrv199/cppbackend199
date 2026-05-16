#pragma once
#include <vector>
#include <cstddef>

struct BagItem {
    size_t id;
    size_t type;
};

class Player {
public:
    Player(size_t bag_capacity) : bag_capacity_(bag_capacity) {}
    
    bool AddItem(size_t id, size_t type) {
        if (bag_.size() >= bag_capacity_) {
            return false;
        }
        bag_.push_back({id, type});
        return true;
    }
    
    size_t DeliverItems() {
        size_t count = bag_.size();
        bag_.clear();
        return count;
    }
    
    const std::vector<BagItem>& GetBag() const { return bag_; }
    size_t GetBagSize() const { return bag_.size(); }
    size_t GetBagCapacity() const { return bag_capacity_; }
    
private:
    size_t bag_capacity_;
    std::vector<BagItem> bag_;
};
