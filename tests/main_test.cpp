#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "data/HistoricCSVDataHandler.h"
#include "events/Event.h"

using namespace hft_system;

// The test fixture now manages the logger's lifecycle.
class ArchitectureTest : public ::testing::Test
{
protected:
    // This runs before each test.
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();
    }

    // This runs after each test.
    void TearDown() override
    {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
}; // <--- THIS SEMICOLON WAS THE CAUSE OF THE COMPILE ERROR

TEST_F(ArchitectureTest, DataHandlerPublishesAndEventBusDispatches)
{
    // 1. Arrange
    std::promise<double> price_promise;
    std::future<double> price_future = price_promise.get_future();

    // 2. Subscribe to MarketEvents
    event_bus->subscribe(EventType::MARKET,
                         [&](const Event &event)
                         {
                             // Use dynamic_cast for safe downcasting to prevent crashes.
                             const auto *market_event = dynamic_cast<const MarketEvent *>(&event);
                             if (market_event)
                             {
                                 if (market_event->symbol == "TEST_BTC")
                                 {
                                     price_promise.set_value(market_event->price);
                                 }
                             }
                         });

    // 3. Create and start the components
    HistoricCSVDataHandler data_handler(event_bus, "TEST_BTC", "data/test_market_data.csv");

    event_bus->start();
    data_handler.start();

    // 4. Act & Assert
    auto future_status = price_future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. Event was published but not received by promise.";
    EXPECT_DOUBLE_EQ(price_future.get(), 16550.50);

    // 5. Cleanup
    data_handler.stop();
    event_bus->stop();
}