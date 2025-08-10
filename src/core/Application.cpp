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

namespace hft_system {

// Constructor now takes a config object directly
Application::Application(Config config)
    : config_(std::move(config)) {
    event_bus_ = std::make_shared<EventBus>();
}

// run_backtest is the primary method for executing a single simulation
std::map<std::string, double> Application::run_backtest() {
    try {
        // --- 1. Create Components ---
        data_handler_ = std::make_shared<HistoricCSVDataHandler>(event_bus_, config_.data.symbol, config_.data.file_path);
        strategy_manager_ = std::make_shared<StrategyManager>(event_bus_, "StrategyManager");
        portfolio_manager_ = std::make_shared<PortfolioManager>(event_bus_, "PortfolioManager", config_.initial_capital);
        risk_manager_ = std::make_shared<RiskManager>(event_bus_, "RiskManager", config_);
        execution_handler_ = std::make_shared<ExecutionHandler>(event_bus_, "ExecutionHandler", config_.execution);
        analytics_ = std::make_shared<Analytics>(event_bus_, "Analytics", config_.analytics);

        // --- 2. Load Strategies ---
        for (const auto& sc : config_.strategies) {
            if (sc.name == "ORDER_BOOK_IMBALANCE") {
                strategy_manager_->add_strategy(std::make_unique<OrderBookImbalanceStrategy>(sc.symbol, sc.params.lookback_levels, sc.params.imbalance_threshold));
            }
        }

        // --- 3. Setup Completion Signal ---
        std::promise<void> promise;
        auto future = promise.get_future();
        event_bus_->subscribe(EventType::SYSTEM, [&](const Event& e) { promise.set_value(); });

        // --- 4. Start System ---
        event_bus_->start();
        portfolio_manager_->start();
        strategy_manager_->start();
        risk_manager_->start();
        execution_handler_->start();
        analytics_->start();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        data_handler_->start();

        // --- 5. Wait for Completion ---
        future.get();

        // --- 6. Shutdown ---
        data_handler_->stop();
        analytics_->stop();
        execution_handler_->stop();
        risk_manager_->stop();
        strategy_manager_->stop();
        portfolio_manager_->stop();
        event_bus_->stop();

        // --- 7. Return Report ---
        return analytics_->generate_report(portfolio_manager_->get_trade_log());

    } catch (const std::exception& e) {
        Log::get_logger()->critical("Exception in backtest run: {}", e.what());
        return {}; // Return empty report on failure
    }
}

} // namespace hft_system