#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "execution/ExecutionHandler.h"
#include "events/Event.h"
#include "config/Config.h" // Include the config header

using namespace hft_system;

class ExecutionHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();

        // **FIX 1:** Create a default config and pass it to the constructor.
        ExecutionConfig config;
        execution_handler = std::make_shared<ExecutionHandler>(event_bus, "TestExecutionHandler", config);
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

    event_bus->subscribe(EventType::FILL,
                         [&](const Event &event)
                         {
                             const auto &fill_event = dynamic_cast<const FillEvent &>(event);
                             EXPECT_EQ(fill_event.symbol, "GOOG");
                             EXPECT_EQ(fill_event.quantity, 50);

                             fill_received_promise.set_value(true);
                         });

    event_bus->start();
    execution_handler->start();

    // 2. Act
    // **FIX 2:** Add the required market_price argument to the OrderEvent constructor.
    double dummy_market_price = 299.85;
    auto order_event = std::make_shared<OrderEvent>("GOOG", OrderDirection::SELL, 50, dummy_market_price);
    event_bus->publish(order_event);

    // 3. Assert
    auto future_status = fill_received_future.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The FillEvent was never received.";

    // Cleanup
    execution_handler->stop();
    event_bus->stop();
}