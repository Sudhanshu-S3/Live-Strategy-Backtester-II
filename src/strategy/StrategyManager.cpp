#include "../../include/strategy/StrategyManager.h"
#include "../../include/strategy/OrderBookImbalanceStrategy.h"
#include "../../include/core/Log.h"
#include <functional>

namespace hft_system
{

    StrategyManager::StrategyManager(std::shared_ptr<EventBus> event_bus, std::string name)
        : Component(event_bus, std::move(name))
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::MARKET, std::bind(&StrategyManager::on_market_event, this, _1));
        event_bus_->subscribe(EventType::ORDER_BOOK, std::bind(&StrategyManager::on_order_book_event, this, _1));
        event_bus_->subscribe(EventType::NEWS, std::bind(&StrategyManager::on_news_event, this, _1));
        // Add subscription for the new event
        event_bus_->subscribe(EventType::MARKET_REGIME_CHANGED, std::bind(&StrategyManager::on_market_regime_event, this, _1));
    }

    void StrategyManager::start() { Log::get_logger()->info("{} started.", name_); }
    void StrategyManager::stop() { Log::get_logger()->info("{} stopped.", name_); }

    void StrategyManager::add_strategy(std::unique_ptr<Strategy> strategy)
    {
        if (strategy)
            strategies_.push_back(std::move(strategy));
    }

    void StrategyManager::on_market_event(const Event &event)
    {
        const auto &market_event = static_cast<const MarketEvent &>(event);
        for (const auto &strategy : strategies_)
        {
            auto signal_event = strategy->calculate_signal(market_event);
            if (signal_event)
            {
                event_bus_->publish(std::move(signal_event));
            }
        }
    }

    void StrategyManager::on_order_book_event(const Event &event)
    {
        const auto &order_book_event = static_cast<const OrderBookEvent &>(event);
        for (const auto &strategy : strategies_)
        {
            if (auto *imbalance_strategy = dynamic_cast<OrderBookImbalanceStrategy *>(strategy.get()))
            {
                auto signal_event = imbalance_strategy->calculate_signal_from_order_book(order_book_event);
                if (signal_event)
                {
                    Log::get_logger()->info("{}: Imbalance strategy generated a signal. Publishing.", name_);
                    event_bus_->publish(std::move(signal_event));
                }
            }
        }
    }
    void StrategyManager::on_news_event(const Event &event)
    {
        const auto &news_event = static_cast<const NewsEvent &>(event);
        for (const auto &strategy : strategies_)
        {
            if (auto *imbalance_strategy = dynamic_cast<OrderBookImbalanceStrategy *>(strategy.get()))
            {
                imbalance_strategy->on_news(news_event);
            }
        }
    }
    void StrategyManager::on_market_regime_event(const Event &event)
    {
        const auto &regime_event = static_cast<const MarketRegimeChangedEvent &>(event);
        for (const auto &strategy : strategies_)
        {
            if (auto *imbalance_strategy = dynamic_cast<OrderBookImbalanceStrategy *>(strategy.get()))
            {
                imbalance_strategy->on_market_regime_change(regime_event.state);
            }
        }
    }

} // namespace hft_system