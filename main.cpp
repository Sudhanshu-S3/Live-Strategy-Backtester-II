#include <iostream>
#include <memory>
#include <future>
#include <stdexcept>

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
#include "include/analytics/MonteCarloSimulator.h"
#include "include/analytics/MarketRegimeDetector.h"
#include "include/data/NewsDataHandler.h"

int main(int argc, char **argv)
{
    hft_system::Log::init();

    try
    {
        hft_system::Config config = hft_system::ConfigParser::parse("config.json");
        hft_system::Log::get_logger()->info("Configuration loaded for run_mode: {}",
                                            config.run_mode == hft_system::RunMode::LIVE ? "LIVE" : "BACKTEST");

        auto event_bus = std::make_shared<hft_system::EventBus>();

        std::shared_ptr<hft_system::DataHandler> data_handler;
        if (config.run_mode == hft_system::RunMode::LIVE)
        {
            data_handler = std::make_shared<hft_system::WebSocketDataHandler>(event_bus, config.websocket);
        }
        else
        {
            data_handler = std::make_shared<hft_system::HistoricCSVDataHandler>(event_bus, config.data.symbol, "tests/data/order_book_data.csv");
        }

        auto strategy_manager = std::make_shared<hft_system::StrategyManager>(event_bus, "StrategyManager");
        auto portfolio_manager = std::make_shared<hft_system::PortfolioManager>(event_bus, "PortfolioManager", config.initial_capital);
        auto risk_manager = std::make_shared<hft_system::RiskManager>(event_bus, "RiskManager", config);
        auto execution_handler = std::make_shared<hft_system::ExecutionHandler>(event_bus, "ExecutionHandler", config.execution);
        auto analytics = std::make_shared<hft_system::Analytics>(event_bus, "Analytics", config.analytics);
        auto news_handler = std::make_shared<hft_system::NewsDataHandler>(event_bus, "NewsHandler");
        auto regime_detector = std::make_shared<hft_system::MarketRegimeDetector>(event_bus, "RegimeDetector", 20, 10);

        for (const auto &strategy_config : config.strategies)
        {
            if (strategy_config.name == "ORDER_BOOK_IMBALANCE")
            {
                strategy_manager->add_strategy(
                    std::make_unique<hft_system::OrderBookImbalanceStrategy>(
                        strategy_config.symbol,
                        strategy_config.params.lookback_levels,
                        strategy_config.params.imbalance_threshold));
            }
        }

        std::promise<void> backtest_complete_promise;
        std::future<void> backtest_complete_future = backtest_complete_promise.get_future();
        event_bus->subscribe(hft_system::EventType::SYSTEM,
                             [&](const hft_system::Event &event)
                             {
                                 backtest_complete_promise.set_value();
                             });

        event_bus->start();
        portfolio_manager->start();
        strategy_manager->start();
        risk_manager->start();
        execution_handler->start();
        analytics->start();
        news_handler->start();
        regime_detector->start();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        data_handler->start();

        if (config.run_mode == hft_system::RunMode::BACKTEST)
        {
            hft_system::Log::get_logger()->info("--- Backtest Running ---");
            backtest_complete_future.get();

            analytics->generate_report(portfolio_manager->get_trade_log());

            const int num_simulations = 1000;
            hft_system::MonteCarloSimulator mc_sim(
                portfolio_manager->get_trade_log(),
                num_simulations,
                config.initial_capital);
            mc_sim.run();
            mc_sim.print_results();
        }
        else
        {
            hft_system::Log::get_logger()->info("--- Live Mode Running ---");
            std::promise<void>().get_future().wait();
        }

        data_handler->stop();
        analytics->stop();
        news_handler->stop();
        regime_detector->stop();
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