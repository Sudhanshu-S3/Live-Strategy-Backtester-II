#include "../../include/core/Application.h"
#include "../../include/core/Log.h"
#include <future>

// Include full definitions of all components
#include "../../include/data/HistoricCSVDataHandler.h"
#include "../../include/data/WebSocketDataHandler.h"
#include "../../include/strategy/StrategyManager.h"
#include "../../include/strategy/OrderBookImbalanceStrategy.h"
#include "../../include/core/PortfolioManager.h"
#include "../../include/risk/RiskManager.h"
#include "../../include/execution/ExecutionHandler.h"
#include "../../include/analytics/Analytics.h"

namespace hft_system
{

    Application::Application(Config config)
        : config_(std::move(config))
    {
        // We only initialize the event bus here. Other components are created in the run loops.
        event_bus_ = std::make_shared<EventBus>();
    }

    Application::~Application()
    {
        stop();
    }

    void Application::run()
    {
        if (!is_running_.load())
        {
            is_running_.store(true);
            app_thread_ = std::thread(&Application::main_loop, this);
        }
    }

    void Application::stop()
    {
        if (is_running_.load())
        {
            is_running_.store(false);
            if (app_thread_.joinable())
            {
                app_thread_.join();
            }
        }
    }

    std::map<std::string, double> Application::get_analytics_report()
    {
        if (analytics_ && portfolio_manager_)
        {
            return analytics_->generate_report(portfolio_manager_->get_trade_log());
        }
        return {};
    }

    // This is a dedicated, blocking function for a single backtest run
    std::map<std::string, double> Application::run_backtest()
    {
        main_loop();
        return get_analytics_report();
    }

    void Application::main_loop()
    {
        try
        {
            Log::get_logger()->info("Application main loop started.");

            if (config_.run_mode == RunMode::LIVE)
            {
                data_handler_ = std::make_shared<WebSocketDataHandler>(event_bus_, config_.websocket);
            }
            else
            {
                data_handler_ = std::make_shared<HistoricCSVDataHandler>(event_bus_, config_.data.symbol, config_.data.file_path);
            }
            strategy_manager_ = std::make_shared<StrategyManager>(event_bus_, "StrategyManager");
            portfolio_manager_ = std::make_shared<PortfolioManager>(event_bus_, "PortfolioManager", config_.initial_capital);

            // **THE FIX IS HERE:** Create the MLModelManager first...
            auto ml_manager = std::make_shared<MLModelManager>(event_bus_, "MLModelManager", config_.machine_learning);
            // ...and then pass it as the 4th argument to the RiskManager.
            risk_manager_ = std::make_shared<RiskManager>(event_bus_, "RiskManager", config_, ml_manager);

            execution_handler_ = std::make_shared<ExecutionHandler>(event_bus_, "ExecutionHandler", config_.execution);
            analytics_ = std::make_shared<Analytics>(event_bus_, "Analytics", config_.analytics);

            for (const auto &sc : config_.strategies)
            {
                if (sc.name == "ORDER_BOOK_IMBALANCE")
                {
                    strategy_manager_->add_strategy(std::make_unique<OrderBookImbalanceStrategy>(sc.symbol, sc.params.lookback_levels, sc.params.imbalance_threshold));
                }
            }

            std::promise<void> promise;
            auto future = promise.get_future();
            event_bus_->subscribe(EventType::SYSTEM, [&](const Event &e)
                                  {
            try { promise.set_value(); } catch (const std::future_error& e) {} });

            event_bus_->start();
            portfolio_manager_->start();
            strategy_manager_->start();
            risk_manager_->start();
            ml_manager->start(); // Also start the new component
            execution_handler_->start();
            analytics_->start();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            data_handler_->start();

            if (config_.run_mode == RunMode::BACKTEST)
            {
                future.get();
            }
            else
            {
                while (is_running_.load())
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }

            data_handler_->stop();
            analytics_->stop();
            execution_handler_->stop();
            risk_manager_->stop();
            ml_manager->stop(); // Stop the new component
            strategy_manager_->stop();
            portfolio_manager_->stop();
            event_bus_->stop();
        }
        catch (const std::exception &e)
        {
            Log::get_logger()->critical("Exception in application main loop: {}", e.what());
        }
        is_running_.store(false);
    }

} // namespace hft_system