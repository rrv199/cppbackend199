#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <atomic>
#include "hotdog.h"
#include "result.h"
#include "gascooker.h"
#include "ingredients.h"

namespace net = boost::asio;

using HotDogHandler = std::function<void(Result<HotDog>)>;

class Cafeteria : public std::enable_shared_from_this<Cafeteria> {
public:
    Cafeteria(net::io_context& io) : io_(io), cooker_(std::make_shared<GasCooker>(io)), next_id_(0) {}

    void OrderHotDog(HotDogHandler handler) {
        net::post(io_, [self = shared_from_this(), handler]() {
            self->DoOrder(handler);
        });
    }

private:
    void DoOrder(HotDogHandler handler) {
        struct State {
            HotDogHandler handler;
            std::shared_ptr<Bread> bread;
            std::shared_ptr<Sausage> sausage;
            bool bread_done = false;
            bool sausage_done = false;
        };
        auto state = std::make_shared<State>();
        state->handler = std::move(handler);
        state->bread = store_.GetBread();
        state->sausage = store_.GetSausage();

        auto check = [this, state]() {
            if (state->bread_done && state->sausage_done) {
                int id = ++next_id_;
                HotDog hd(id, state->sausage, state->bread);
                state->handler(Result<HotDog>(std::move(hd)));
            }
        };

        state->bread->StartBake(cooker_, [state, check]() {
            state->bread_done = true;
            check();
        });

        state->sausage->StartFry(cooker_, [state, check]() {
            state->sausage_done = true;
            check();
        });
    }

    net::io_context& io_;
    Store store_;
    std::shared_ptr<GasCooker> cooker_;
    std::atomic<int> next_id_;
};
