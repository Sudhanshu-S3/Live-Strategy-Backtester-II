// include/events/Event.h
#ifndef HFT_SYSTEM_EVENT_H
#define HFT_SYSTEM_EVENT_H

#include <string>
#include <memory>
#include <chrono>

namespace hft_system {

// Forward-declare data types that events might use.
// This reduces header dependencies. We'll include the full DataTypes.h in the .cpp files.
enum class OrderDirection;

// Defines the core types of events that drive the system.
enum class EventType {
    MARKET,     // A market data update (tick, bar)
    SIGNAL,     // A trading signal from a Strategy
    ORDER,      // A request to place an order
    FILL,       // Confirmation that an order has been filled
    SYSTEM      // For internal system communication (e.g., shutdown)
};

// Base class for all events.
// Using polymorphism with a virtual destructor allows us to pass around
// different event types as a pointer to the base class.
struct Event {
    explicit Event(EventType type) : type(type) {
        // Record the time the event was created
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    virtual ~Event() = default;

    const EventType type;
    long long timestamp; // Nanoseconds since epoch
};


// === Specific Event Definitions ===

struct MarketEvent : public Event {
    MarketEvent(std::string symbol, double price)
        : Event(EventType::MARKET), symbol(std::move(symbol)), price(price) {}

    const std::string symbol;
    const double price;
    // We can add more fields like volume, bid, ask later.
};

struct SignalEvent : public Event {
    SignalEvent(std::string symbol, OrderDirection direction, std::string strategy_id)
        : Event(EventType::SIGNAL), symbol(std::move(symbol)),
          direction(direction), strategy_id(std::move(strategy_id)) {}
    
    const std::string symbol;
    const OrderDirection direction;
    const std::string strategy_id;
};

// We will add OrderEvent, FillEvent, etc., as we build the components that need them.

} // namespace hft_system

#endif // HFT_SYSTEM_EVENT_H