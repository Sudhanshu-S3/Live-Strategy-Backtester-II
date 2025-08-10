#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <chrono>
#include <string>

#include "core/Log.h"
#include "core/EventBus.h"
#include "config/Config.h"
#include "data/HistoricCSVDataHandler.h"
#include "strategy/StrategyManager.h"
#include "strategy/BuyEveryTickStrategy.h"
#include "core/PortfolioManager.h"
#include "risk/RiskManager.h"
#include "execution/ExecutionHandler.h"
#include "events/Event.h"

using namespace hft_system;
using namespace std::chrono_literals;

class IntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();

        config.initial_capital = 100000.0;
        config.data.symbol = "TEST_STOCK";
        // Construct the full path
        config.data.file_path = std::string(PROJECT_SOURCE_DIR) + "/tests/data/test_market_data.csv";
        config.risk.risk_per_trade_pct = 0.20;

        data_handler = std::make_shared<HistoricCSVDataHandler>(event_bus, config.data.symbol, config.data.file_path);
        strategy_manager = std::make_shared<StrategyManager>(event_bus, "StrategyManager");
        portfolio_manager = std::make_shared<PortfolioManager>(event_bus, "PortfolioManager", config.initial_capital);
        risk_manager = std::make_shared<RiskManager>(event_bus, "RiskManager", config);
        execution_handler = std::make_shared<ExecutionHandler>(event_bus, "ExecutionHandler", config.execution);
    }

    void TearDown() override
    {
        Log::shutdown();
    }

    Config config;
    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<HistoricCSVDataHandler> data_handler;
    std::shared_ptr<StrategyManager> strategy_manager;
    std::shared_ptr<PortfolioManager> portfolio_manager;
    std::shared_ptr<RiskManager> risk_manager;
    std::shared_ptr<ExecutionHandler> execution_handler;
};

TEST_F(IntegrationTest, FullEndToEndLoop)
{
    std::promise<void> fill_received_promise;
    std::future<void> fill_received_future = fill_received_promise.get_future();

    strategy_manager->add_strategy(std::make_unique<BuyEveryTickStrategy>());

    event_bus->subscribe(EventType::FILL,
                         [&](const Event &event)
                         {
                             fill_received_promise.set_value();
                         });

    event_bus->start();
    portfolio_manager->start();
    strategy_manager->start();
    risk_manager->start();
    execution_handler->start();
    data_handler->start();

    auto future_status = fill_received_future.wait_for(2s);

    data_handler->stop();
    execution_handler->stop();
    risk_manager->stop();
    strategy_manager->stop();
    portfolio_manager->stop();
    event_bus->stop();

    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out.";
}