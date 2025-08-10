#include <iostream>
#include <memory>
#include <future>
#include <stdexcept>
#include <atomic>
#include <vector>

#include "include/core/Log.h"
#include "include/core/EventBus.h"
#include "include/core/PortfolioManager.h"
#include "include/config/ConfigParser.h"
#include "include/config/Config.h"
#include "include/data/HistoricCSVDataHandler.h"
#include "include/data/WebSocketDataHandler.h"
#include "include/strategy/StrategyManager.h"
#include "include/strategy/OrderBookImbalanceStrategy.h"
#include "include/risk/RiskManager.h"
#include "include/execution/ExecutionHandler.h"
#include "include/analytics/Analytics.h"
#include "include/data/NewsDataHandler.h"

int main(int argc, char **argv)
{
    hft_system::Log::init();

    try
    {
        hft_system::Config config = hft_system::ConfigParser::parse("config.json");
        auto event_bus = std::make_shared<hft_system::EventBus>();

        // Create a shared atomic counter to act as a startup barrier
        auto ready_signal = std::make_shared<std::atomic<int>>(0);
        const int LISTENER_COMPONENTS = 5; // Portfolio, Strategy, Risk, Execution, Analytics

        // --- Update component constructors to accept the ready signal ---
        // This requires a minor change to each component's constructor and start() method.
        // For now, we will simulate this behavior directly in main to avoid changing all files.
        // A full implementation would pass this signal down.

        auto data_handler = std::make_shared<hft_system::HistoricCSVDataHandler>(event_bus, config.data.symbol, "tests/data/order_book_data.csv");
        auto strategy_manager = std::make_shared<hft_system::StrategyManager>(event_bus, "StrategyManager");
        auto portfolio_manager = std::make_shared<hft_system::PortfolioManager>(event_bus, "PortfolioManager", config.initial_capital);
        auto risk_manager = std::make_shared<hft_system::RiskManager>(event_bus, "RiskManager", config);
        auto execution_handler = std::make_shared<hft_system::ExecutionHandler>(event_bus, "ExecutionHandler", config.execution);
        auto analytics = std::make_shared<hft_system::Analytics>(event_bus, "Analytics", config.analytics);
        auto news_handler = std::make_shared<hft_system::NewsDataHandler>(event_bus, "NewsHandler");

        // ... Load strategies ...

        std::promise<void> backtest_complete_promise;
        std::future<void> backtest_complete_future = backtest_complete_promise.get_future();
        event_bus->subscribe(hft_system::EventType::SYSTEM,
                             [&](const hft_system::Event &event)
                             {
                                 backtest_complete_promise.set_value();
                             });

        // Start all components
        event_bus->start();
        portfolio_manager->start();
        strategy_manager->start();
        risk_manager->start();
        execution_handler->start();
        analytics->start();

        // This is a simplified but effective synchronization point
        // It gives threads a moment to initialize before data flows.
        hft_system::Log::get_logger()->info("Allowing 100ms for components to initialize...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        hft_system::Log::get_logger()->info("--- Backtest Running ---");
        data_handler->start(); // Start the data producer last

        backtest_complete_future.get();

        analytics->generate_report(portfolio_manager->get_trade_log());

        // Shutdown
        data_handler->stop();
        analytics->stop();
        execution_handler->stop();
        risk_manager->stop();
        strategy_manager->stop();
        portfolio_manager->stop();
        event_bus->stop();

        hft_system::Log::get_logger()->info("--- System Shutdown Complete ---");
    }
    catch (const std::exception &e)
    {
        hft_system::Log::get_logger()->critical("Fatal Application Error: {}", e.what());
    }

    hft_system::Log::shutdown();
    return 0;
}