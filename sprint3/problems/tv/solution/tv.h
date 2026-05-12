#pragma once
#include <optional>
#include <iostream>

class TV {
public:
    TV();
    
    bool IsTurnedOn() const noexcept;
    void TurnOn() noexcept;
    void TurnOff() noexcept;
    std::optional<int> GetChannel() const noexcept;
    void SelectChannel(int channel) noexcept;
    void SelectPreviousChannel() noexcept;

private:
    bool is_on_;
    int current_channel_;
    int previous_channel_;
};
