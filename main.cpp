#include "include/core/Application.h"
#include "include/api/APIServer.h"
#include <iostream>
#include <future> // Add this include for std::promise

int main()
{
    hft_system::Application app;

    // Add the missing IP address argument ("0.0.0.0" to listen on all interfaces)
    hft_system::APIServer api_server(app, "0.0.0.0", 3000);

    api_server.start();

    std::cout << "HFT System initialized. API server is running." << std::endl;
    std::cout << "Use API to start/stop the backtester. Press Ctrl+C to exit." << std::endl;

    // This line keeps the main thread alive so the server can run.
    std::promise<void>().get_future().wait();

    return 0;
}