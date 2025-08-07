
#ifndef HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H
#define HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H

#include "Strategy.h"
#include <string>

namespace hft_system {

class OrderBookImbalanceStrategy : public Strategy {
public:
    OrderBookImbalanceStrategy(std::string symbol, int levels, double threshold);

    std::unique_ptr<SignalEvent> calculate_signal(const MarketEvent& event) override;
    std::unique_ptr<SignalEvent> calculate_signal_from_order_book(const OrderBookEvent& event);

private:
    std::string symbol_;
    int lookback_levels_;
    double imbalance_threshold_;
};

} // namespace hft_system
#endif // HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H