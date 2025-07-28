// main.cpp
#include <iostream>
#include <memory>
#include <chrono>
#include "include/core/EventBus.h" // Note the path

int main(int argc, char **argv)
{
    std::cout << "System starting up..." << std::endl;

    auto event_bus = std::make_shared<hft_system::EventBus>();
    event_bus->start();

    std::cout << "EventBus is running. System will shut down in 5 seconds." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    event_bus->stop();
    std::cout << "System shut down." << std::endl;

    return 0;
}