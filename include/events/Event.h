// include/events/Event.h
#ifndef HFT_SYSTEM_EVENT_H
#define HFT_SYSTEM_EVENT_H

#include "../core/DataTypes.h" // Must include the correct definitions
#include <string>
#include <memory>
#include <chrono>

namespace hft_system {

enum class EventType {
    MARKET,
    SIGNAL,
    ORDER,
    FILL,
    SYSTEM
};

struct Event {
    explicit Event(EventType type) : type(type) {
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    virtual ~Event() = default;

    const EventType type;
    long long timestamp;
};

struct MarketEvent : public Event {
    MarketEvent(std::string symbol, double price)
        : Event(EventType::MARKET), symbol(std::move(symbol)), price(price) {}

    const std::string symbol;
    const double price;
};

struct SignalEvent : public Event {
    SignalEvent(std::string symbol, OrderDirection direction)
        : Event(EventType::SIGNAL), symbol(std::move(symbol)), direction(direction) {}
    
    const std::string symbol;
    const OrderDirection direction;
};

struct OrderEvent : public Event {
    OrderEvent(std::string symbol, OrderDirection direction, int quantity)
        : Event(EventType::ORDER), symbol(std::move(symbol)), direction(direction), quantity(quantity) {}

    const std::string symbol;
    const OrderDirection direction;
    const int quantity;
};

struct FillEvent : public Event {
    FillEvent(std::string symbol, OrderDirection direction, int quantity, double fill_price)
        : Event(EventType::FILL), symbol(std::move(symbol)), direction(direction), 
          quantity(quantity), fill_price(fill_price) {}

    const std::string symbol;
    const OrderDirection direction;
    const int quantity;
    const double fill_price;
};

} // namespace hft_system
#endif // HFT_SYSTEM_EVENT_H