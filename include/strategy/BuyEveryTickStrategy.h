#ifndef HFT_SYSTEM_BUYEVERYTICKSTRATEGY_H
#define HFT_SYSTEM_BUYEVERYTICKSTRATEGY_H

#include "Strategy.h"

namespace hft_system {

// A simple test strategy that generates a BUY signal on every market tick.
class BuyEveryTickStrategy : public Strategy {
public:
    std::unique_ptr<SignalEvent> calculate_signal(const MarketEvent& event) override;
};

} // namespace hft_system

#endif // HFT_SYSTEM_BUYEVERYTICKSTRATEGY_H