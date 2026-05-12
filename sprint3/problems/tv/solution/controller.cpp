#include "controller.h"
#include <iostream>
#include <string>
#include <sstream>

Controller::Controller() : tv_(std::make_unique<TV>()), menu_(std::make_unique<Menu>()), running_(true) {
    SetupMenu();
}

void Controller::SetupMenu() {
    menu_->AddCommand("TurnOn", "Turn on the TV", [this]() { TurnOn(); });
    menu_->AddCommand("TurnOff", "Turn off the TV", [this]() { TurnOff(); });
    menu_->AddCommand("Info", "Show TV info", [this]() { Info(); });
    menu_->AddCommand("SelectPreviousChannel", "Switch to previous channel", [this]() { SelectPreviousChannel(); });
    menu_->AddCommand("Help", "Show help", [this]() { menu_->PrintHelp(); });
    menu_->AddCommand("Exit", "Exit program", [this]() { Exit(); });
}

void Controller::TurnOn() {
    tv_->TurnOn();
    std::cout << "TV is turned on\n";
}

void Controller::TurnOff() {
    tv_->TurnOff();
    std::cout << "TV is turned off\n";
}

void Controller::Info() const {
    if (tv_->IsTurnedOn()) {
        std::cout << "TV is ON, channel: " << tv_->GetChannel().value() << "\n";
    } else {
        std::cout << "TV is OFF\n";
    }
}

void Controller::SelectChannel(int channel) {
    tv_->SelectChannel(channel);
    std::cout << "Channel selected: " << channel << "\n";
}

void Controller::SelectPreviousChannel() {
    tv_->SelectPreviousChannel();
    std::cout << "Switched to previous channel\n";
}

void Controller::Exit() {
    running_ = false;
    std::cout << "Goodbye!\n";
}

void Controller::Run() {
    std::cout << "Welcome to TV Controller!\n";
    menu_->PrintHelp();
    
    std::string input;
    while (running_) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input.find("SelectChannel") == 0) {
            std::istringstream iss(input);
            std::string cmd;
            int channel;
            iss >> cmd >> channel;
            if (iss) {
                SelectChannel(channel);
            } else {
                std::cout << "Invalid command. Use: SelectChannel <number>\n";
            }
        } else if (!menu_->ExecuteCommand(input)) {
            std::cout << "Unknown command. Type 'Help' for available commands.\n";
        }
    }
}
