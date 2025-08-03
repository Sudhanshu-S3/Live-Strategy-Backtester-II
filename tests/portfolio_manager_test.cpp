// tests/portfolio_manager_test.cpp
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "core/PortfolioManager.h"
#include "events/Event.h"

using namespace hft_system;

// Test fixture for the PortfolioManager
class PortfolioManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();
        portfolio_manager = std::make_shared<PortfolioManager>(event_bus, "TestPortfolio", 100000.0);
    }

    void TearDown() override
    {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<PortfolioManager> portfolio_manager;
};

TEST_F(PortfolioManagerTest, ShouldGenerateOrderEventOnSignal)
{
    // 1. Arrange
    std::promise<bool> order_received_promise;
    std::future<bool> order_received_future = order_received_promise.get_future();

    // Our "Test Spy" that listens for the OrderEvent
    event_bus->subscribe(EventType::ORDER,
                         [&](const Event &event)
                         {
                             const auto &order_event = dynamic_cast<const OrderEvent &>(event);
                             // Verify the order is what we expect
                             EXPECT_EQ(order_event.symbol, "AAPL");
                             EXPECT_EQ(order_event.direction, OrderDirection::BUY);
                             EXPECT_EQ(order_event.quantity, 100);

                             // Fulfill the promise to signal the test has passed
                             order_received_promise.set_value(true);
                         });

    event_bus->start();
    portfolio_manager->start();

    // 2. Act
    // Manually create and publish a signal, as if it came from a StrategyManager
    auto signal = std::make_shared<SignalEvent>("AAPL", OrderDirection::BUY);
    event_bus->publish(signal);

    // 3. Assert
    // Wait for our spy to receive the order event
    auto future_status = order_received_future.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The OrderEvent was never received.";

    // Cleanup
    portfolio_manager->stop();
    event_bus->stop();
}