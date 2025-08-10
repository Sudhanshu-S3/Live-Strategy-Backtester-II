#include "../../include/core/PortfolioManager.h"
#include "../../include/core/Log.h"
#include <functional>
#include <algorithm>
#include <numeric>

using hft_system::OrderDirection;

namespace hft_system
{

    PortfolioManager::PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital)
        : Component(event_bus, std::move(name)), capital_(initial_capital), cash_(initial_capital)
    {
        using namespace std::placeholders;
        event_bus_->subscribe(EventType::FILL, std::bind(&PortfolioManager::on_fill, this, _1));
    }

    void PortfolioManager::start()
    {
        Log::get_logger()->info("{} started.", name_);
        auto initial_update = std::make_shared<PortfolioUpdateEvent>(capital_, cash_);
        event_bus_->publish(initial_update);
    }

    void PortfolioManager::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void PortfolioManager::on_fill(const Event &event)
    {
        const auto &fill = static_cast<const FillEvent &>(event);

        double cost = fill.fill_price * fill.quantity;
        if (fill.direction == OrderDirection::BUY)
        {
            cash_ -= (cost + fill.commission);
        }
        else
        {
            cash_ += (cost - fill.commission);
        }

        Position &position = positions_[fill.symbol];
        bool is_closing_trade = (position.quantity != 0) && (fill.direction != position.direction);

        if (is_closing_trade)
        {
            Trade trade;
            trade.symbol = fill.symbol;
            trade.direction = position.direction;
            trade.quantity = std::min(position.quantity, fill.quantity);
            trade.entry_price = position.entry_price;
            trade.exit_price = fill.fill_price;

            if (trade.direction == OrderDirection::BUY)
            {
                trade.pnl = (trade.exit_price - trade.entry_price) * trade.quantity - fill.commission;
            }
            else
            {
                trade.pnl = (trade.entry_price - trade.exit_price) * trade.quantity - fill.commission;
            }
            trade_log_.push_back(trade);
            Log::get_logger()->info("Closed trade for {}. P&L: ${:.2f}", trade.symbol, trade.pnl);

            position.quantity -= trade.quantity;
            if (position.quantity < 1e-9) // Check for near-zero quantity
            {
                positions_.erase(fill.symbol);
            }
        }
        else
        {
            double total_value = (position.entry_price * position.quantity) + cost;
            position.quantity += fill.quantity;
            position.entry_price = total_value / position.quantity;
            position.direction = fill.direction;
            position.symbol = fill.symbol;
        }

        double open_positions_value = 0;
        for (const auto &[symbol, pos] : positions_)
        {
            open_positions_value += pos.quantity * pos.entry_price;
        }
        double total_equity = cash_ + open_positions_value;

        auto update_event = std::make_shared<PortfolioUpdateEvent>(total_equity, cash_);
        event_bus_->publish(update_event);
    }

    // **THE FIX IS HERE:** Add the missing function definition.
    const std::list<hft_system::Trade> &PortfolioManager::get_trade_log() const
    {
        return trade_log_;
    }

    std::map<std::string, double> PortfolioManager::get_pnl_summary() const
    {
        std::map<std::string, double> summary;
        double total_pnl = 0.0;
        for (const auto &trade : trade_log_)
        {
            total_pnl += trade.pnl;
        }
        summary["total_pnl"] = total_pnl;
        summary["total_trades"] = trade_log_.size();
        return summary;
    }

} // namespace hft_system