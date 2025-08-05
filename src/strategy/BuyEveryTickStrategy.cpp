#include "../../include/strategy/BuyEveryTickStrategy.h"

namespace hft_system {

std::unique_ptr<SignalEvent> BuyEveryTickStrategy::calculate_signal(const MarketEvent& event) {
    // For any market event, immediately create and return a BUY signal event.
    return std::make_unique<SignalEvent>(event.symbol, OrderDirection::BUY);
}

} // namespace hft_system