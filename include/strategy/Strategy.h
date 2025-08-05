
#ifndef HFT_SYSTEM_STRATEGY_H
#define HFT_SYSTEM_STRATEGY_H

#include "../events/Event.h"
#include <memory> // For std::unique_ptr

namespace hft_system {

// Abstract base class for all trading strategies.
class Strategy {
public:
    virtual ~Strategy() = default;

    // Each strategy must implement this method.
    // It takes market data and returns a signal if conditions are met.
    virtual std::unique_ptr<SignalEvent> calculate_signal(const MarketEvent& event) = 0;
};

} // namespace hft_system

#endif // HFT_SYSTEM_STRATEGY_H