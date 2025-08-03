// tests/execution_handler_test.cpp
#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "execution/ExecutionHandler.h"
#include "events/Event.h"

using namespace hft_system;

class ExecutionHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();
        execution_handler = std::make_shared<ExecutionHandler>(event_bus, "TestExecutionHandler");
    }

    void TearDown() override
    {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<ExecutionHandler> execution_handler;
};

TEST_F(ExecutionHandlerTest, ShouldGenerateFillEventOnOrderEvent)
{
    // 1. Arrange
    std::promise<bool> fill_received_promise;
    std::future<bool> fill_received_future = fill_received_promise.get_future();

    // Subscribe a "spy" to listen for the resulting FillEvent
    event_bus->subscribe(EventType::FILL,
                         [&](const Event &event)
                         {
                             const auto &fill_event = dynamic_cast<const FillEvent &>(event);
                             EXPECT_EQ(fill_event.symbol, "GOOG");
                             EXPECT_EQ(fill_event.quantity, 50);
                             EXPECT_EQ(fill_event.direction, OrderDirection::SELL);
                             EXPECT_DOUBLE_EQ(fill_event.fill_price, 300.0); // Matches the simulated price

                             fill_received_promise.set_value(true);
                         });

    event_bus->start();
    execution_handler->start();

    // 2. Act
    // Manually publish an OrderEvent, as if it came from a PortfolioManager
    auto order_event = std::make_shared<OrderEvent>("GOOG", OrderDirection::SELL, 50);
    event_bus->publish(order_event);

    // 3. Assert
    auto future_status = fill_received_future.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The FillEvent was never received.";

    // Cleanup
    execution_handler->stop();
    event_bus->stop();
}