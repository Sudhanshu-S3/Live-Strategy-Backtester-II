#include "../../include/strategy/OrderBookImbalanceStrategy.h"
#include "../../include/core/Log.h"
#include <numeric>
#include <algorithm>

namespace hft_system {

OrderBookImbalanceStrategy::OrderBookImbalanceStrategy(std::string symbol, int levels, double threshold)
    : symbol_(std::move(symbol)), lookback_levels_(levels), imbalance_threshold_(threshold) {}

std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal(const MarketEvent& event) {
    return nullptr; // This strategy only uses order book data
}

std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal_from_order_book(const OrderBookEvent& event) {
    if (event.book.symbol != symbol_) return nullptr;
    const auto& bids = event.book.bids;
    const auto& asks = event.book.asks;
    if (bids.empty() || asks.empty()) return nullptr;

    int levels_to_process = std::min({(int)bids.size(), (int)asks.size(), lookback_levels_});

    double total_bid_volume = 0.0;
    for (int i = 0; i < levels_to_process; ++i) total_bid_volume += bids[i].quantity;

    double total_ask_volume = 0.0;
    for (int i = 0; i < levels_to_process; ++i) total_ask_volume += asks[i].quantity;

    if (total_ask_volume <= 1e-9) return nullptr;

    double imbalance_ratio = total_bid_volume / total_ask_volume;
    Log::get_logger()->trace("Imbalance for {}: {:.4f}", symbol_, imbalance_ratio);

    if (imbalance_ratio > imbalance_threshold_) {
        return std::make_unique<SignalEvent>(symbol_, OrderDirection::BUY);
    } else if (imbalance_ratio < (1.0 / imbalance_threshold_)) {
        return std::make_unique<SignalEvent>(symbol_, OrderDirection::SELL);
    }

    return nullptr;
}

} // namespace hft_system