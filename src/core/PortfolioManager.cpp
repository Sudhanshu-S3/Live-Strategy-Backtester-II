#include "../../include/core/PortfolioManager.h"
#include "../../include/core/Log.h"
#include <functional>

using hft_system::OrderDirection;

namespace hft_system
{

    PortfolioManager::PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital)
        : Component(event_bus, std::move(name)), capital_(initial_capital), cash_(initial_capital)
    {

        using namespace std::placeholders;
        // The PortfolioManager now only needs to listen for FillEvents.
        event_bus_->subscribe(EventType::FILL, std::bind(&PortfolioManager::on_fill, this, _1));
    }

    void PortfolioManager::start()
    {
        Log::get_logger()->info("{} started.", name_);
    }

    void PortfolioManager::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void PortfolioManager::on_fill(const Event &event)
    {
        const auto &fill = static_cast<const FillEvent &>(event);
        Log::get_logger()->info("{}: Received fill for {} {} {} at ${}. Commission: ${}",
                                name_, fill.direction == OrderDirection::BUY ? "BUY" : "SELL",
                                fill.quantity, fill.symbol, fill.fill_price, fill.commission);

        // Update our cash based on the trade, including commission
        double cost = fill.fill_price * fill.quantity;
        if (fill.direction == OrderDirection::BUY)
        {
            cash_ -= (cost + fill.commission);
        }
        else
        {
            cash_ += (cost - fill.commission);
        }

        Log::get_logger()->info("{}: Cash updated to ${}", name_, cash_);

        // After updating state, publish the new portfolio status.
        // A full implementation would add the market value of all positions to cash_ to get total_equity.
        double total_equity = cash_;
        auto update_event = std::make_shared<PortfolioUpdateEvent>(total_equity, cash_);
        event_bus_->publish(update_event);
    }

} // namespace hft_system