#ifndef HFT_SYSTEM_EVENT_H
#define HFT_SYSTEM_EVENT_H

#include "../core/DataTypes.h"
#include <string>
#include <memory>
#include <chrono>

namespace hft_system
{

    enum class EventType
    {
        MARKET,
        ORDER_BOOK, // New event for order book updates
        SIGNAL,
        ORDER,
        FILL,
        PORTFOLIO_UPDATE,
        SYSTEM
    };

    struct Event
    {
        explicit Event(EventType type) : type(type)
        {
            timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
        }
        virtual ~Event() = default;

        const EventType type;
        long long timestamp;
    };

    struct MarketEvent : public Event
    {
        MarketEvent(std::string symbol, double price)
            : Event(EventType::MARKET), symbol(std::move(symbol)), price(price) {}

        const std::string symbol;
        const double price;
    };

    struct OrderBookEvent : public Event
    {
        OrderBookEvent(OrderBook book)
            : Event(EventType::ORDER_BOOK), book(std::move(book)) {}
        const OrderBook book;
    };

    struct SignalEvent : public Event
    {
        SignalEvent(std::string symbol, OrderDirection direction)
            : Event(EventType::SIGNAL), symbol(std::move(symbol)), direction(direction) {}

        const std::string symbol;
        const OrderDirection direction;
    };

    struct OrderEvent : public Event
    {
        OrderEvent(std::string symbol, OrderDirection direction, int quantity, double market_price) // Add market_price
            : Event(EventType::ORDER), symbol(std::move(symbol)), direction(direction),
              quantity(quantity), market_price(market_price)
        {
        } // Add market_price

        const std::string symbol;
        const OrderDirection direction;
        const int quantity;
        const double market_price; // ADD THIS LINE
    };

    struct FillEvent : public Event
    {
        FillEvent(std::string symbol, OrderDirection direction, int quantity, double fill_price, double commission) // Add commission
            : Event(EventType::FILL), symbol(std::move(symbol)), direction(direction),
              quantity(quantity), fill_price(fill_price), commission(commission)
        {
        } // Add commission

        const std::string symbol;
        const OrderDirection direction;
        const int quantity;
        const double fill_price;
        const double commission; // ADD THIS LINE
    };

    struct PortfolioUpdateEvent : public Event
    {
        PortfolioUpdateEvent(double equity, double cash)
            : Event(EventType::PORTFOLIO_UPDATE), total_equity(equity), cash(cash) {}

        const double total_equity;
        const double cash;
    };
} // namespace hft_system
#endif // HFT_SYSTEM_EVENT_H