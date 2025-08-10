#ifndef HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H
#define HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H

#include "Strategy.h"
#include "../events/Event.h" // Include for MarketState
#include <string>
#include <map>

namespace hft_system {

class OrderBookImbalanceStrategy : public Strategy {
public:
    OrderBookImbalanceStrategy(std::string symbol, int levels, double threshold);

    std::unique_ptr<SignalEvent> calculate_signal(const MarketEvent& event) override;
    std::unique_ptr<SignalEvent> calculate_signal_from_order_book(const OrderBookEvent& event);
    void on_news(const NewsEvent& event);

    // Add a handler for market regime events
    void on_market_regime_change(const MarketState& new_state);

private:
    std::string symbol_;
    int lookback_levels_;
    double base_imbalance_threshold_; // Base threshold
    double current_imbalance_threshold_; // Adjusted threshold
    std::map<std::string, double> sentiment_scores_;
};

} // namespace hft_system
#endif // HFT_SYSTEM_ORDERBOOKIMBALANCESTRATEGY_H