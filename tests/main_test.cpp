#include <gtest/gtest.h>
#include <future>
#include <memory>
#include <string>

#include "core/Log.h"
#include "core/EventBus.h"
#include "data/HistoricCSVDataHandler.h"
#include "events/Event.h"

using namespace hft_system;

class ArchitectureTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
        event_bus = std::make_shared<EventBus>();
    }
    void TearDown() override
    {
        Log::shutdown();
    }
    std::shared_ptr<EventBus> event_bus;
};

TEST_F(ArchitectureTest, DataHandlerPublishesAndEventBusDispatches)
{
    std::promise<double> price_promise;
    std::future<double> price_future = price_promise.get_future();

    event_bus->subscribe(EventType::MARKET,
                         [&](const Event &event)
                         {
                             const auto *market_event = dynamic_cast<const MarketEvent *>(&event);
                             if (market_event && market_event->symbol == "TEST_BTC")
                             {
                                 price_promise.set_value(market_event->price);
                             }
                         });

    // Construct the full path to the data file
    std::string data_file_path = std::string(PROJECT_SOURCE_DIR) + "/tests/data/test_market_data.csv";
    HistoricCSVDataHandler data_handler(event_bus, "TEST_BTC", data_file_path);

    event_bus->start();
    data_handler.start();

    auto future_status = price_future.wait_for(std::chrono::seconds(2));

    data_handler.stop();
    event_bus->stop();

    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out.";
    EXPECT_DOUBLE_EQ(price_future.get(), 16550.50);
}