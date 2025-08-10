#include "../../include/strategy/OrderBookImbalanceStrategy.h"
#include "../../include/core/Log.h"
#include <numeric>
#include <algorithm>

namespace hft_system {

OrderBookImbalanceStrategy::OrderBookImbalanceStrategy(std::string symbol, int levels, double threshold)
    : symbol_(std::move(symbol)), lookback_levels_(levels), imbalance_threshold_(threshold) {}

std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal(const MarketEvent& event) {
    return nullptr;
}

// Add the implementation for the news handler
void OrderBookImbalanceStrategy::on_news(const NewsEvent& event) {
    Log::get_logger()->info("Strategy for {} received news with sentiment {:.2f}", symbol_, event.sentiment_score);
    sentiment_scores_[event.symbol] = event.sentiment_score;
}

std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal_from_order_book(const OrderBookEvent& event) {
    if (event.book.symbol != symbol_) return nullptr;
    
    // --- (Imbalance calculation logic is the same as before) ---
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
    
    // --- NEW LOGIC: Check sentiment before sending a signal ---
    double current_sentiment = sentiment_scores_.count(symbol_) ? sentiment_scores_[symbol_] : 0.0;
    
    if (imbalance_ratio > imbalance_threshold_) { // Potential BUY signal
        if (current_sentiment < -0.5) {
            Log::get_logger()->info("BUY signal for {} blocked by strong negative sentiment ({:.2f})", symbol_, current_sentiment);
            return nullptr;
        }
        return std::make_unique<SignalEvent>(symbol_, OrderDirection::BUY);

    } else if (imbalance_ratio < (1.0 / imbalance_threshold_)) { // Potential SELL signal
        if (current_sentiment > 0.5) {
            Log::get_logger()->info("SELL signal for {} blocked by strong positive sentiment ({:.2f})", symbol_, current_sentiment);
            return nullptr;
        }
        return std::make_unique<SignalEvent>(symbol_, OrderDirection::SELL);
    }

    return nullptr;
}

} // namespace hft_system