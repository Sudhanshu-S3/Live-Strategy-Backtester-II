#include "../../include/risk/RiskManager.h"
#include "../../include/core/Log.h"
#include <functional>

namespace hft_system
{

    RiskManager::RiskManager(std::shared_ptr<EventBus> event_bus, std::string name, const Config &config, std::shared_ptr<MLModelManager> ml_manager)
        : Component(event_bus, std::move(name)),
          risk_config_(config.risk),
          ml_manager_(ml_manager),
          latest_equity_(config.initial_capital),
          latest_cash_(config.initial_capital)
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::SIGNAL, std::bind(&RiskManager::on_signal, this, _1));
        event_bus_->subscribe(EventType::MARKET, std::bind(&RiskManager::on_market, this, _1));
        event_bus_->subscribe(EventType::PORTFOLIO_UPDATE, std::bind(&RiskManager::on_portfolio_update, this, _1));
    }

    void RiskManager::start() { Log::get_logger()->info("{} started.", name_); }
    void RiskManager::stop() { Log::get_logger()->info("{} stopped.", name_); }

    void RiskManager::on_market(const Event &event)
    {
        const auto &market = static_cast<const MarketEvent &>(event);
        latest_prices_[market.symbol] = market.price;
    }

    void RiskManager::on_portfolio_update(const Event &event)
    {
        const auto &update = static_cast<const PortfolioUpdateEvent &>(event);
        latest_equity_ = update.total_equity;
        latest_cash_ = update.cash;
        Log::get_logger()->trace("{}: Cached portfolio state updated. Equity: ${}, Cash: ${}", name_, latest_equity_, latest_cash_);
    }

    void RiskManager::on_signal(const Event &event)
    {
        const auto &signal = static_cast<const SignalEvent &>(event);
        if (latest_prices_.find(signal.symbol) == latest_prices_.end())
        {
            return; // No market price available yet
        }
        double market_price = latest_prices_.at(signal.symbol);

        // --- NEW DYNAMIC SIZING LOGIC ---
        double base_risk_amount = latest_equity_ * risk_config_.risk_per_trade_pct;
        double final_risk_amount = base_risk_amount;

        if (risk_config_.use_dynamic_sizing && ml_manager_)
        {
            double confidence = ml_manager_->get_trade_confidence(signal.symbol);
            Log::get_logger()->info("{}: ML Model confidence for {} is {:.2f}", name_, signal.symbol, confidence);
            // Adjust risk based on model confidence (e.g., take less risk on low-confidence trades)
            final_risk_amount *= confidence;
        }

        int quantity = static_cast<int>(final_risk_amount / market_price);
        // --- END NEW LOGIC ---

        if (quantity <= 0)
            return;
        if (quantity * market_price > latest_cash_)
            return;

        Log::get_logger()->info("{}: Signal for {} approved. Dynamically sized to {} units.", name_, signal.symbol, quantity);
        auto order = std::make_shared<OrderEvent>(signal.symbol, signal.direction, quantity, market_price);
        event_bus_->publish(order);
    }

} // namespace hft_system