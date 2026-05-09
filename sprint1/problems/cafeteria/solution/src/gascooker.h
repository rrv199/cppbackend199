#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>

namespace net = boost::asio;

class GasCooker : public std::enable_shared_from_this<GasCooker> {
public:
    using Handler = std::function<void()>;

    GasCooker(net::io_context& io, int num_burners = 8)
        : io_(io)
        , free_burners_(num_burners) {
    }

    net::io_context& GetIoContext() { return io_; }

    void UseBurner(Handler handler) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (free_burners_ > 0) {
                --free_burners_;
                net::post(io_, std::move(handler));
                return;
            }
            pending_handlers_.push(std::move(handler));
        }
    }

    void ReleaseBurner() {
        Handler handler;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++free_burners_;
            if (!pending_handlers_.empty()) {
                handler = std::move(pending_handlers_.front());
                pending_handlers_.pop();
                --free_burners_;
            }
        }
        if (handler) {
            net::post(io_, std::move(handler));
        }
    }

private:
    net::io_context& io_;
    int free_burners_;
    std::queue<Handler> pending_handlers_;
    std::mutex mutex_;
};
