// include/core/PortfolioManager.h
#ifndef HFT_SYSTEM_PORTFOLIOMANAGER_H
#define HFT_SYSTEM_PORTFOLIOMANAGER_H

#include "../core/Component.h"
#include "../events/Event.h"

namespace hft_system {

// Manages the portfolio's state, including positions, cash, and equity.
// Subscribes to SignalEvents and FillEvents. Publishes OrderEvents.
class PortfolioManager : public Component {
public:
    PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital)
        : Component(event_bus, std::move(name)), capital_(initial_capital) {}
    
private:
    // Callback for when a strategy generates a trading signal.
    void on_signal_event(const Event& event);

    // Callback for when an order is confirmed as filled by the execution handler.
    void on_fill_event(const Event& event);

    double capital_;
    // Internal state like positions, cash, etc., will be private members.
};

} // namespace hft_system

#endif // HFT_SYSTEM_PORTFOLIOMANAGER_H