#include "../../include/analytics/MarketRegimeDetector.h"
#include "../../include/core/Log.h"
#include <functional>
#include <numeric>
#include <cmath>

namespace hft_system
{

    MarketRegimeDetector::MarketRegimeDetector(std::shared_ptr<EventBus> event_bus, std::string name, int volatility_period, int trend_period)
        : Component(event_bus, std::move(name)), volatility_period_(volatility_period), trend_period_(trend_period) {}

    void MarketRegimeDetector::start()
    {
        using namespace std::placeholders;
        event_bus_->subscribe(EventType::MARKET, std::bind(&MarketRegimeDetector::on_market_event, this, _1));
        Log::get_logger()->info("{} started.", name_);
    }

    void MarketRegimeDetector::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void MarketRegimeDetector::on_market_event(const Event &event)
    {
        const auto &market = static_cast<const MarketEvent &>(event);
        price_history_.push_back(market.price);

        // Keep the deque at the max size we need
        if (price_history_.size() > std::max(volatility_period_, trend_period_))
        {
            price_history_.pop_front();
        }

        // Calculate the new regime, but only if we have enough data
        if (price_history_.size() >= std::max(volatility_period_, trend_period_))
        {
            calculate_regime();
        }
    }

    void MarketRegimeDetector::calculate_regime()
    {
        // 1. Calculate Volatility (Standard Deviation of returns)
        std::vector<double> returns;
        for (size_t i = price_history_.size() - volatility_period_; i < price_history_.size() - 1; ++i)
        {
            returns.push_back(price_history_[i + 1] / price_history_[i] - 1.0);
        }
        double sum = std::accumulate(returns.begin(), returns.end(), 0.0);
        double mean = sum / returns.size();
        double sq_sum = std::inner_product(returns.begin(), returns.end(), returns.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / returns.size() - mean * mean);

        // 2. Calculate Trend (Simple Moving Average slope)
        double first_sma = std::accumulate(price_history_.end() - trend_period_, price_history_.end() - (trend_period_ / 2), 0.0) / (trend_period_ / 2);
        double second_sma = std::accumulate(price_history_.end() - (trend_period_ / 2), price_history_.end(), 0.0) / (trend_period_ / 2);

        MarketState new_state;
        // Determine Volatility Regime
        if (stdev > 0.02)
            new_state.volatility = MarketState::Volatility::HIGH;
        else if (stdev < 0.005)
            new_state.volatility = MarketState::Volatility::LOW;
        else
            new_state.volatility = MarketState::Volatility::NORMAL;

        // Determine Trend Regime
        if (second_sma > first_sma * 1.01)
            new_state.trend = MarketState::Trend::TRENDING_UP;
        else if (second_sma < first_sma * 0.99)
            new_state.trend = MarketState::Trend::TRENDING_DOWN;
        else
            new_state.trend = MarketState::Trend::SIDEWAYS;

        // If the state has changed, publish an event
        if (new_state.volatility != current_state_.volatility || new_state.trend != current_state_.trend)
        {
            current_state_ = new_state;
            Log::get_logger()->info("New Market Regime Detected: Volatility={}, Trend={}",
                                    (int)current_state_.volatility, (int)current_state_.trend);
            auto regime_event = std::make_shared<MarketRegimeChangedEvent>(current_state_);
            event_bus_->publish(regime_event);
        }
    }

} // namespace hft_system