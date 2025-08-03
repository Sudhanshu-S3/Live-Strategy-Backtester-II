// tests/strategy_manager_test.cpp
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "strategy/StrategyManager.h"
#include "strategy/BuyEveryTickStrategy.h"
#include "events/Event.h"

using namespace hft_system;

class StrategyManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init();
        event_bus = std::make_shared<EventBus>();
        strategy_manager = std::make_shared<StrategyManager>(event_bus, "TestStrategyManager");
    }

    void TearDown() override {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<StrategyManager> strategy_manager;
};

TEST_F(StrategyManagerTest, ShouldGenerateSignalEventOnMarketEvent) {
    // 1. Arrange
    std::promise<bool> signal_received_promise;
    std::future<bool> signal_received_future = signal_received_promise.get_future();

    // Add our simple strategy to the manager
    strategy_manager->add_strategy(std::make_unique<BuyEveryTickStrategy>());

    // Subscribe a "spy" to listen for the resulting SignalEvent
    event_bus->subscribe(EventType::SIGNAL,
        [&](const Event& event) {
            const auto& signal_event = dynamic_cast<const SignalEvent&>(event);
            EXPECT_EQ(signal_event.symbol, "MSFT");
            EXPECT_EQ(signal_event.direction, OrderDirection::BUY);
            signal_received_promise.set_value(true);
        });

    event_bus->start();
    strategy_manager->start();

    // 2. Act
    // Manually publish a MarketEvent, as if it came from a DataHandler
    auto market_event = std::make_shared<MarketEvent>("MSFT", 300.50);
    event_bus->publish(market_event);

    // 3. Assert
    auto future_status = signal_received_future.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The SignalEvent was never received.";

    // Cleanup
    strategy_manager->stop();
    event_bus->stop();
}