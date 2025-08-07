#include "../../include/execution/ExecutionHandler.h"
#include "../../include/config/Config.h"
#include "../../include/core/Log.h"
#include <functional>

using hft_system::OrderDirection;

namespace hft_system
{

    ExecutionHandler::ExecutionHandler(std::shared_ptr<EventBus> event_bus, std::string name, const ExecutionConfig &config)
        : Component(event_bus, std::move(name)), config_(config)
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::ORDER, std::bind(&ExecutionHandler::on_order, this, _1));
    }

    void ExecutionHandler::start()
    {
        Log::get_logger()->info("{} started.", name_);
    }

    void ExecutionHandler::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void ExecutionHandler::on_order(const Event &event)
    {
        const auto &order = static_cast<const OrderEvent &>(event);
        Log::get_logger()->info("{}: Received order to {} {} {}.", name_,
                                order.direction == OrderDirection::BUY ? "BUY" : "SELL",
                                order.quantity, order.symbol);

        double slippage = order.market_price * config_.slippage_pct;
        double fill_price = 0.0;

        if (order.direction == OrderDirection::BUY)
        {
            fill_price = order.market_price + slippage;
        }
        else
        {
            fill_price = order.market_price - slippage;
        }

        double commission = fill_price * order.quantity * config_.commission_pct;

        auto fill = std::make_shared<FillEvent>(
            order.symbol,
            order.direction,
            order.quantity,
            fill_price,
            commission);

        event_bus_->publish(fill);
        Log::get_logger()->info("{}: Published fill event for {}. Fill Price (with slippage): ${}, Commission: ${}",
                                name_, order.symbol, fill_price, commission);
    }

} // namespace hft_system