#include "../../include/risk/RiskManager.h"
#include "../../include/core/Log.h"
#include <functional>
#include <algorithm>

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
        event_bus_->subscribe(EventType::ORDER_BOOK, std::bind(&RiskManager::on_order_book, this, _1));
    }

    void RiskManager::start() { Log::get_logger()->info("{} started.", name_); }
    void RiskManager::stop() { Log::get_logger()->info("{} stopped.", name_); }

    void RiskManager::on_market(const Event &event)
    {
        const auto &market = static_cast<const MarketEvent &>(event);
        latest_prices_[market.symbol] = market.price;
    }

    void RiskManager::on_order_book(const Event &event)
    {
        const auto &order_book = static_cast<const OrderBookEvent &>(event);
        if (!order_book.book.bids.empty())
        {
            // Use the best bid price as the current market price
            latest_prices_[order_book.book.symbol] = order_book.book.bids[0].price;
        }
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
            Log::get_logger()->warn("{}: Rejecting signal for {}. No market price available.", name_, signal.symbol);
            return;
        }

        double market_price = latest_prices_.at(signal.symbol);

        // Calculate base risk amount
        double base_risk_amount = latest_equity_ * risk_config_.risk_per_trade_pct;
        double final_risk_amount = base_risk_amount;

        // Apply ML confidence if enabled
        if (risk_config_.use_dynamic_sizing && ml_manager_)
        {
            double confidence = ml_manager_->get_trade_confidence(signal.symbol);
            Log::get_logger()->debug("{}: ML Model confidence for {} is {:.2f}", name_, signal.symbol, confidence);
            final_risk_amount *= confidence;
        }

        // Calculate quantity as double first, then apply minimum constraints
        double raw_quantity = final_risk_amount / market_price;

        // Define minimum order requirements (these should ideally come from config)
        const double MIN_BTC_QUANTITY = 0.001; // Binance minimum for BTC
        const double MIN_ORDER_VALUE = 10.0;   // $10 minimum order value

        // Apply minimum quantity constraint
        double quantity = std::max(raw_quantity, MIN_BTC_QUANTITY);

        // Check if order meets minimum value requirement
        double order_value = quantity * market_price;
        if (order_value < MIN_ORDER_VALUE)
        {
            quantity = MIN_ORDER_VALUE / market_price;
            order_value = quantity * market_price;
        }

        // Debug logging to help troubleshoot
        Log::get_logger()->debug("{}: Risk calculation for {}: Equity=${:.2f}, Risk%={:.4f}, RiskAmount=${:.2f}, Price=${:.2f}, RawQty={:.6f}, FinalQty={:.6f}, OrderValue=${:.2f}",
                                 name_, signal.symbol, latest_equity_, risk_config_.risk_per_trade_pct,
                                 final_risk_amount, market_price, raw_quantity, quantity, order_value);

        // Final validation checks
        if (quantity <= 0)
        {
            Log::get_logger()->warn("{}: Calculated quantity is zero or negative for {}. No order generated.", name_, signal.symbol);
            return;
        }

        if (order_value > latest_cash_)
        {
            Log::get_logger()->warn("{}: Rejecting signal for {}. Insufficient cash. Required: ${:.2f}, Available: ${:.2f}",
                                    name_, signal.symbol, order_value, latest_cash_);
            return;
        }

        // Optional: Check against maximum position size if you add it to config later
        // const double MAX_POSITION_VALUE = 5000.0;  // $5000 max position
        // if (order_value > MAX_POSITION_VALUE) {
        //     quantity = MAX_POSITION_VALUE / market_price;
        //     order_value = quantity * market_price;
        //     Log::get_logger()->info("{}: Order size reduced due to max position limit. New quantity: {:.6f}", name_, quantity);
        // }

        Log::get_logger()->info("{}: Signal for {} approved. Quantity: {:.6f} units, Value: ${:.2f}",
                                name_, signal.symbol, quantity, order_value);

        // Create order with double precision quantity
        auto order = std::make_shared<OrderEvent>(signal.symbol, signal.direction, quantity, market_price);
        event_bus_->publish(order);
    }

}