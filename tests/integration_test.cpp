// tests/integration_test.cpp
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "data/HistoricCSVDataHandler.h"
#include "strategy/StrategyManager.h"
#include "strategy/BuyEveryTickStrategy.h"
#include "core/PortfolioManager.h"
#include "execution/ExecutionHandler.h"
#include "events/Event.h"

using namespace hft_system;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init();
        event_bus = std::make_shared<EventBus>();

        // Create all components
        data_handler = std::make_shared<HistoricCSVDataHandler>(event_bus, "TEST_STOCK", "data/test_market_data.csv");
        strategy_manager = std::make_shared<StrategyManager>(event_bus, "StrategyManager");
        portfolio_manager = std::make_shared<PortfolioManager>(event_bus, "PortfolioManager", 100000.0);
        execution_handler = std::make_shared<ExecutionHandler>(event_bus, "ExecutionHandler");
    }

    void TearDown() override {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<HistoricCSVDataHandler> data_handler;
    std::shared_ptr<StrategyManager> strategy_manager;
    std::shared_ptr<PortfolioManager> portfolio_manager;
    std::shared_ptr<ExecutionHandler> execution_handler;
};

TEST_F(IntegrationTest, FullEndToEndLoop) {
    // 1. Arrange
    std::promise<bool> fill_received_promise;
    std::future<bool> fill_received_future = fill_received_promise.get_future();

    // Add our simple strategy to the manager
    strategy_manager->add_strategy(std::make_unique<BuyEveryTickStrategy>());

    // Subscribe a "spy" to the very last event in the chain: the FillEvent
    event_bus->subscribe(EventType::FILL,
        [&](const Event& event) {
            // If we receive this event, it means the entire chain worked.
            const auto& fill_event = dynamic_cast<const FillEvent&>(event);
            EXPECT_EQ(fill_event.symbol, "TEST_STOCK");
            EXPECT_EQ(fill_event.direction, OrderDirection::BUY);
            fill_received_promise.set_value(true);
        });

    // 2. Act
    // Start all components and the event bus
    event_bus->start();
    portfolio_manager->start();
    strategy_manager->start();
    execution_handler->start();
    data_handler->start(); // The DataHandler kicks off the entire event cascade

    // 3. Assert
    // Wait for the FillEvent to be received
    auto future_status = fill_received_future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The FillEvent was never received.";

    // Cleanup
    data_handler->stop();
    execution_handler->stop();
    strategy_manager->stop();
    portfolio_manager->stop();
    event_bus->stop();
}