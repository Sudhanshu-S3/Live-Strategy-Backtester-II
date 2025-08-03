// main.cpp
#include <iostream>
#include <memory>
#include <future>

#include "include/core/Log.h"
#include "include/core/EventBus.h"
#include "include/data/HistoricCSVDataHandler.h"
#include "include/strategy/StrategyManager.h"
#include "include/strategy/BuyEveryTickStrategy.h"
#include "include/core/PortfolioManager.h"
#include "include/execution/ExecutionHandler.h"

int main(int argc, char **argv)
{
    // 1. Initialization
    hft_system::Log::init();
    auto event_bus = std::make_shared<hft_system::EventBus>();

    // Create all components
    auto data_handler = std::make_shared<hft_system::HistoricCSVDataHandler>(event_bus, "TEST_STOCK", "tests/data/test_market_data.csv");
    auto strategy_manager = std::make_shared<hft_system::StrategyManager>(event_bus, "StrategyManager");
    auto portfolio_manager = std::make_shared<hft_system::PortfolioManager>(event_bus, "PortfolioManager", 100000.0);
    auto execution_handler = std::make_shared<hft_system::ExecutionHandler>(event_bus, "ExecutionHandler");

    // Load our simple strategy
    strategy_manager->add_strategy(std::make_unique<hft_system::BuyEveryTickStrategy>());

    // 2. Setup Completion Signal
    std::promise<void> backtest_complete_promise;
    std::future<void> backtest_complete_future = backtest_complete_promise.get_future();

    // Subscribe to the system event that signals the end of the backtest
    event_bus->subscribe(hft_system::EventType::SYSTEM,
                         [&](const hft_system::Event &event)
                         {
                             hft_system::Log::get_logger()->info("--- Backtest Complete Signal Received ---");
                             backtest_complete_promise.set_value();
                         });

    // 3. Start the System
    event_bus->start();
    portfolio_manager->start();
    strategy_manager->start();
    execution_handler->start();
    data_handler->start(); // DataHandler starts last to kick off the event flow

    hft_system::Log::get_logger()->info("--- Backtest Running ---");

    // 4. Wait for Completion
    // The main thread will block here until the promise is fulfilled.
    backtest_complete_future.get();

    // 5. Shutdown
    data_handler->stop();
    execution_handler->stop();
    strategy_manager->stop();
    portfolio_manager->stop();
    event_bus->stop();

    hft_system::Log::get_logger()->info("--- System Shutdown Complete ---");
    hft_system::Log::shutdown();

    return 0;
}