#include "tv.h"

TV::TV() : is_on_(false), current_channel_(1), previous_channel_(1) {}

bool TV::IsTurnedOn() const noexcept {
    return is_on_;
}

void TV::TurnOn() noexcept {
    is_on_ = true;
    current_channel_ = previous_channel_;
}

void TV::TurnOff() noexcept {
    is_on_ = false;
    previous_channel_ = current_channel_;
}

std::optional<int> TV::GetChannel() const noexcept {
    if (!is_on_) {
        return std::nullopt;
    }
    return current_channel_;
}

void TV::SelectChannel(int channel) noexcept {
    if (!is_on_) return;
    if (channel < 1 || channel > 999) return;
    previous_channel_ = current_channel_;
    current_channel_ = channel;
}

void TV::SelectPreviousChannel() noexcept {
    if (!is_on_) return;
    std::swap(current_channel_, previous_channel_);
}
