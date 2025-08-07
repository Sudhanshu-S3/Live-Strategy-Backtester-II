#include <gtest/gtest.h>
#include <future>
#include <memory>

#include "core/Log.h"
#include "core/EventBus.h"
#include "core/PortfolioManager.h"
#include "events/Event.h"

using namespace hft_system;

class PortfolioManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init();
        event_bus = std::make_shared<EventBus>();
        portfolio_manager = std::make_shared<PortfolioManager>(event_bus, "TestPortfolio", 100000.0);
    }

    void TearDown() override {
        Log::shutdown();
    }

    std::shared_ptr<EventBus> event_bus;
    std::shared_ptr<PortfolioManager> portfolio_manager;
};

// This test now verifies that the PortfolioManager correctly
// processes a FillEvent and emits a PortfolioUpdateEvent.
TEST_F(PortfolioManagerTest, ShouldProcessFillEventAndUpdateState) {
    // 1. Arrange
    std::promise<bool> update_received_promise;
    std::future<bool> update_received_future = update_received_promise.get_future();

    // Our spy now listens for the PortfolioUpdateEvent
    event_bus->subscribe(EventType::PORTFOLIO_UPDATE,
        [&](const Event& event) {
            const auto& update_event = dynamic_cast<const PortfolioUpdateEvent&>(event);
            // Initial capital was 100,000. Fill was for 10 * 150.25 = 1502.5. Commission was 1.5.
            // New cash should be 100000 - 1502.5 - 1.5 = 98496.0
            EXPECT_NEAR(update_event.cash, 98496.0, 0.01);
            update_received_promise.set_value(true);
        });
    
    event_bus->start();
    portfolio_manager->start();

    // 2. Act
    // Manually publish a FillEvent, as if from the ExecutionHandler
    auto fill_event = std::make_shared<FillEvent>("AAPL", OrderDirection::BUY, 10, 150.25, 1.50);
    event_bus->publish(fill_event);

    // 3. Assert
    auto future_status = update_received_future.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(future_status, std::future_status::ready) << "Test timed out. The PortfolioUpdateEvent was never received.";

    // Cleanup
    portfolio_manager->stop();
    event_bus->stop();
}