#include "../../include/core/PortfolioManager.h"
#include "../../include/core/Log.h"
#include <functional>

// By explicitly declaring that we are "using" this specific type,
// we can resolve the compiler's confusion.
using hft_system::OrderDirection;

namespace hft_system
{

    PortfolioManager::PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital)
        : Component(event_bus, std::move(name)), capital_(initial_capital), cash_(initial_capital)
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::SIGNAL, std::bind(&PortfolioManager::on_signal, this, _1));
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

    void PortfolioManager::on_signal(const Event &event)
    {
        const auto &signal = static_cast<const SignalEvent &>(event);

        // Now we can use the simpler form because of the 'using' directive above
        Log::get_logger()->info("{}: Received signal to {} {}.", name_,
                                signal.direction == OrderDirection::BUY ? "BUY" : "SELL", signal.symbol);

        int quantity_to_trade = 100;

        auto order = std::make_shared<OrderEvent>(signal.symbol, signal.direction, quantity_to_trade);
        event_bus_->publish(order);
        Log::get_logger()->info("{}: Published order event for {}.", name_, signal.symbol);
    }

    void PortfolioManager::on_fill(const Event &event)
    {
        const auto &fill = static_cast<const FillEvent &>(event);

        Log::get_logger()->info("{}: Received fill for {} {} {} at ${}",
                                name_, fill.direction == OrderDirection::BUY ? "BUY" : "SELL",
                                fill.quantity, fill.symbol, fill.fill_price);

        double cost = fill.fill_price * fill.quantity;
        if (fill.direction == OrderDirection::BUY)
        {
            cash_ -= cost;
        }
        else
        {
            cash_ += cost;
        }

        Log::get_logger()->info("{}: Cash updated to ${}", name_, cash_);
    }

} // namespace hft_system