#include "../../include/strategy/OrderBookImbalanceStrategy.h"
#include "../../include/core/Log.h"
#include <numeric>
#include <algorithm>

namespace hft_system
{

    OrderBookImbalanceStrategy::OrderBookImbalanceStrategy(std::string symbol, int levels, double threshold)
        : symbol_(std::move(symbol)),
          lookback_levels_(levels),
          base_imbalance_threshold_(threshold),
          current_imbalance_threshold_(threshold) {}

    std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal(const MarketEvent &event)
    {
        return nullptr;
    }

    void OrderBookImbalanceStrategy::on_news(const NewsEvent &event)
    {
        sentiment_scores_[event.symbol] = event.sentiment_score;
    }

    // NEW METHOD: React to market regime changes
    void OrderBookImbalanceStrategy::on_market_regime_change(const MarketState &new_state)
    {
        if (new_state.volatility == MarketState::Volatility::HIGH)
        {
            // In high volatility, be more conservative (require a stronger signal)
            current_imbalance_threshold_ = base_imbalance_threshold_ * 1.5;
            Log::get_logger()->info("Strategy {}: High volatility detected. Threshold increased to {:.2f}", symbol_, current_imbalance_threshold_);
        }
        else
        {
            // In normal or low volatility, use the base threshold
            current_imbalance_threshold_ = base_imbalance_threshold_;
            Log::get_logger()->info("Strategy {}: Volatility normalized. Threshold reset to {:.2f}", symbol_, current_imbalance_threshold_);
        }
    }

    std::unique_ptr<SignalEvent> OrderBookImbalanceStrategy::calculate_signal_from_order_book(const OrderBookEvent &event)
    {
        if (event.book.symbol != symbol_)
            return nullptr;

        // ... (Imbalance calculation is the same as before) ...
        const auto &bids = event.book.bids;
        const auto &asks = event.book.asks;
        if (bids.empty() || asks.empty())
            return nullptr;
        int levels_to_process = std::min({(int)bids.size(), (int)asks.size(), lookback_levels_});
        double total_bid_volume = 0.0;
        for (int i = 0; i < levels_to_process; ++i)
            total_bid_volume += bids[i].quantity;
        double total_ask_volume = 0.0;
        for (int i = 0; i < levels_to_process; ++i)
            total_ask_volume += asks[i].quantity;
        if (total_ask_volume <= 1e-9)
            return nullptr;
        double imbalance_ratio = total_bid_volume / total_ask_volume;

        double current_sentiment = sentiment_scores_.count(symbol_) ? sentiment_scores_[symbol_] : 0.0;

        // Use the CURRENT (adjusted) threshold for decisions
        if (imbalance_ratio > current_imbalance_threshold_)
        {
            if (current_sentiment < -0.5)
                return nullptr;
            return std::make_unique<SignalEvent>(symbol_, OrderDirection::BUY);
        }
        else if (imbalance_ratio < (1.0 / current_imbalance_threshold_))
        {
            if (current_sentiment > 0.5)
                return nullptr;
            return std::make_unique<SignalEvent>(symbol_, OrderDirection::SELL);
        }

        return nullptr;
    }

} // namespace hft_system