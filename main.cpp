#include <iostream>
#include <memory>
#include <future>
#include <stdexcept>

#include "include/config/ConfigParser.h"
#include "include/config/Config.h"

#include "include/core/Log.h"
#include "include/core/EventBus.h"
#include "include/data/HistoricCSVDataHandler.h"
#include "include/strategy/StrategyManager.h"
#include "include/strategy/BuyEveryTickStrategy.h"
#include "include/core/PortfolioManager.h"
#include "include/execution/ExecutionHandler.h"

int main(int argc, char **argv)
{
    hft_system::Log::init();

    try
    {
        // 1. Load Configuration
        hft_system::Config config = hft_system::ConfigParser::parse("config.json");
        hft_system::Log::get_logger()->info("Configuration loaded successfully.");
        hft_system::Log::get_logger()->info("Initial Capital: ${}", config.initial_capital);
        hft_system::Log::get_logger()->info("Data Symbol: {}", config.data.symbol);

        // 2. Initialization
        auto event_bus = std::make_shared<hft_system::EventBus>();

        // Create all components using values from the config object
        auto data_handler = std::make_shared<hft_system::HistoricCSVDataHandler>(event_bus, config.data.symbol, config.data.file_path);
        auto strategy_manager = std::make_shared<hft_system::StrategyManager>(event_bus, "StrategyManager");
        auto portfolio_manager = std::make_shared<hft_system::PortfolioManager>(event_bus, "PortfolioManager", config.initial_capital);
        auto execution_handler = std::make_shared<hft_system::ExecutionHandler>(event_bus, "ExecutionHandler");

        // ... (rest of the main function is the same as before)
        strategy_manager->add_strategy(std::make_unique<hft_system::BuyEveryTickStrategy>());
        // ...

        std::promise<void> backtest_complete_promise;
        std::future<void> backtest_complete_future = backtest_complete_promise.get_future();

        event_bus->subscribe(hft_system::EventType::SYSTEM,
                             [&](const hft_system::Event &event)
                             {
                                 hft_system::Log::get_logger()->info("--- Backtest Complete Signal Received ---");
                                 backtest_complete_promise.set_value();
                             });

        event_bus->start();
        portfolio_manager->start();
        strategy_manager->start();
        execution_handler->start();
        data_handler->start();

        hft_system::Log::get_logger()->info("--- Backtest Running ---");
        backtest_complete_future.get();

        data_handler->stop();
        execution_handler->stop();
        strategy_manager->stop();
        portfolio_manager->stop();
        event_bus->stop();

        hft_system::Log::get_logger()->info("--- System Shutdown Complete ---");
    }
    catch (const std::exception &e)
    {
        hft_system::Log::get_logger()->critical("Fatal Error: {}", e.what());
    }

    hft_system::Log::shutdown();
    return 0;
}