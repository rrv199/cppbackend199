#pragma once

#include "clock.h"
#include "gascooker.h"

#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>

namespace net = boost::asio;

class Sausage : public std::enable_shared_from_this<Sausage> {
public:
    using Handler = std::function<void()>;

    explicit Sausage(int id) : id_(id) {}

    int GetId() const { return id_; }

    void StartFry(std::shared_ptr<GasCooker> cooker, Handler handler) {
        auto self = shared_from_this();
        cooker->UseBurner([this, handler, cooker, self]() mutable {
            start_time_ = Clock::now();
            auto timer = std::make_shared<net::steady_timer>(cooker->GetIoContext());
            timer->expires_after(std::chrono::milliseconds(1500));
            timer->async_wait([this, handler, timer, cooker, self](auto) {
                end_time_ = Clock::now();
                cooker->ReleaseBurner();
                handler();
            });
        });
    }

    bool IsCooked() const { return start_time_.has_value() && end_time_.has_value(); }

    auto GetCookDuration() const {
        if (!IsCooked()) throw std::logic_error("Not cooked");
        return *end_time_ - *start_time_;
    }

private:
    int id_;
    std::optional<Clock::time_point> start_time_;
    std::optional<Clock::time_point> end_time_;
};

class Bread : public std::enable_shared_from_this<Bread> {
public:
    using Handler = std::function<void()>;

    explicit Bread(int id) : id_(id) {}

    int GetId() const { return id_; }

    void StartBake(std::shared_ptr<GasCooker> cooker, Handler handler) {
        auto self = shared_from_this();
        cooker->UseBurner([this, handler, cooker, self]() mutable {
            start_time_ = Clock::now();
            auto timer = std::make_shared<net::steady_timer>(cooker->GetIoContext());
            timer->expires_after(std::chrono::milliseconds(1000));
            timer->async_wait([this, handler, timer, cooker, self](auto) {
                end_time_ = Clock::now();
                cooker->ReleaseBurner();
                handler();
            });
        });
    }

    bool IsCooked() const { return start_time_.has_value() && end_time_.has_value(); }

    auto GetBakingDuration() const {
        if (!IsCooked()) throw std::logic_error("Not baked");
        return *end_time_ - *start_time_;
    }

private:
    int id_;
    std::optional<Clock::time_point> start_time_;
    std::optional<Clock::time_point> end_time_;
};

class Store {
public:
    std::shared_ptr<Bread> GetBread() { return std::make_shared<Bread>(++next_id_); }
    std::shared_ptr<Sausage> GetSausage() { return std::make_shared<Sausage>(++next_id_); }
private:
    int next_id_ = 0;
};
