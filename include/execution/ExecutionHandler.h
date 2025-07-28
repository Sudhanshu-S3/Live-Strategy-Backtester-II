// include/execution/ExecutionHandler.h
#ifndef HFT_SYSTEM_EXECUTIONHANDLER_H
#define HFT_SYSTEM_EXECUTIONHANDLER_H

#include "../core/Component.h"
#include "../events/Event.h"

namespace hft_system {

// Simulates the connection to a brokerage or exchange.
// Subscribes to OrderEvents and publishes FillEvents.
class ExecutionHandler : public Component {
public:
    ExecutionHandler(std::shared_ptr<EventBus> event_bus, std::string name)
        : Component(event_bus, std::move(name)) {}

private:
    // Callback for when the portfolio manager wants to place an order.
    void on_order_event(const Event& event);
};

} // namespace hft_system

#endif // HFT_SYSTEM_EXECUTIONHANDLER_H