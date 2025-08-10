#include "include/core/Application.h"
#include "include/core/Log.h"
#include "include/config/ConfigParser.h"
#include <iostream>
#include <future>
#include <string>

int main()
{
    hft_system::Log::init();
    try
    {
        // **THE FIX IS HERE:** Create an absolute path to the config file.
        std::string config_path = std::string(PROJECT_SOURCE_DIR) + "/config.json";
        hft_system::Log::get_logger()->info("Loading configuration from: {}", config_path);

        hft_system::Config config = hft_system::ConfigParser::parse(config_path);

        hft_system::Application app(config);

        app.run();

        std::cout << "HFT System is running. Press Ctrl+C to exit." << std::endl;

        std::promise<void>().get_future().wait();
    }
    catch (const std::exception &e)
    {
        hft_system::Log::get_logger()->critical("Fatal Application Error: {}", e.what());
    }

    hft_system::Log::shutdown();
    return 0;
}