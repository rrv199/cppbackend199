#pragma once
#include "tv.h"
#include "menu.h"
#include <memory>

class Controller {
public:
    Controller();
    void Run();

private:
    std::unique_ptr<TV> tv_;
    std::unique_ptr<Menu> menu_;
    
    void SetupMenu();
    void TurnOn();
    void TurnOff();
    void Info() const;
    void SelectChannel(int channel);
    void SelectPreviousChannel();
    void Exit();
    
    bool running_;
};
