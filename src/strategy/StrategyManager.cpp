// src/strategy/StrategyManager.cpp
#include "../../include/strategy/StrategyManager.h"
#include "../../include/core/Log.h"
#include <functional>

namespace hft_system
{

    StrategyManager::StrategyManager(std::shared_ptr<EventBus> event_bus, std::string name)
        : Component(event_bus, std::move(name))
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::MARKET, std::bind(&StrategyManager::on_market_event, this, _1));
    }

    void StrategyManager::start()
    {
        Log::get_logger()->info("{} started.", name_);
    }
    void StrategyManager::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void StrategyManager::add_strategy(std::unique_ptr<Strategy> strategy)
    {
        if (strategy)
        {
            strategies_.push_back(std::move(strategy));
        }
    }

    void StrategyManager::on_market_event(const Event &event)
    {
        const auto &market_event = static_cast<const MarketEvent &>(event);
        Log::get_logger()->trace("{}: Received market event for {}", name_, market_event.symbol);

        // Pass the market event to all strategies
        for (const auto &strategy : strategies_)
        {
            auto signal_event = strategy->calculate_signal(market_event);
            if (signal_event)
            {
                // If a strategy generated a signal, publish it
                Log::get_logger()->info("{}: Strategy generated a signal. Publishing.", name_);
                event_bus_->publish(std::move(signal_event));
            }
        }
    }

} // namespace hft_system