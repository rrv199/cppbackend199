#pragma once

class TV {
public:
    TV();
    
    bool IsTurnedOn() const;
    void TurnOn();
    void TurnOff();
    int GetChannel() const;
    void SelectChannel(int channel);

private:
    bool is_on_;
    int current_channel_;
    int last_channel_;
};
