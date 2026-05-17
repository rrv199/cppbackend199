#pragma once

#include <string>

class Player {
public:
    Player(const std::string& name) : name_(name), score_(0), speed_(0.0), idle_time_(0.0), total_play_time_(0.0) {}

    void Update(double delta_time, double speed) {
        total_play_time_ += delta_time;
        speed_ = speed;
        
        if (speed == 0.0) {
            idle_time_ += delta_time;
        } else {
            idle_time_ = 0.0;
        }
    }
    
    void AddScore(int points) {
        score_ += points;
    }
    
    bool IsIdle(double retirement_time) const {
        return idle_time_ >= retirement_time;
    }
    
    double GetTotalPlayTime() const { return total_play_time_; }
    int GetScore() const { return score_; }
    std::string GetName() const { return name_; }
    double GetSpeed() const { return speed_; }

private:
    std::string name_;
    int score_;
    double speed_;
    double idle_time_;
    double total_play_time_;
};
