// include/strategy/StrategyManager.h
#ifndef HFT_SYSTEM_STRATEGYMANAGER_H
#define HFT_SYSTEM_STRATEGYMANAGER_H

#include "../core/Component.h"
#include "../events/Event.h"

namespace hft_system
{

    // Manages one or more trading strategies.
    // Subscribes to MarketEvents and publishes SignalEvents.
    class StrategyManager : public Component
    {
    public:
        StrategyManager(std::shared_ptr<EventBus> event_bus, std::string name)
            : Component(event_bus, std::move(name)) {}

        // Method to load strategies from a configuration.
        void load_strategies(); // Implementation will be in the .cpp file

    private:
        // Callback function that gets triggered by the EventBus for MarketEvents.
        void on_market_event(const Event &event);
    };

} // namespace hft_system

#endif // HFT_SYSTEM_STRATEGYMANAGER_H