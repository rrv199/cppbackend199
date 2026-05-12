#include "tv.h"

TV::TV() : is_on_(false), current_channel_(1), last_channel_(1) {}

bool TV::IsTurnedOn() const {
    return is_on_;
}

void TV::TurnOn() {
    is_on_ = true;
    current_channel_ = last_channel_;
}

void TV::TurnOff() {
    is_on_ = false;
    last_channel_ = current_channel_;
}

int TV::GetChannel() const {
    return current_channel_;
}

void TV::SelectChannel(int channel) {
    if (!is_on_) return;
    if (channel < 1 || channel > 999) return;
    current_channel_ = channel;
}
