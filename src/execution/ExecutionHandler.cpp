#include "../../include/execution/ExecutionHandler.h"
#include "../../include/core/Log.h"
#include <functional>

namespace hft_system {

ExecutionHandler::ExecutionHandler(std::shared_ptr<EventBus> event_bus, std::string name)
    : Component(event_bus, std::move(name)) {
    
    using namespace std::placeholders;
    event_bus_->subscribe(EventType::ORDER, std::bind(&ExecutionHandler::on_order, this, _1));
}

void ExecutionHandler::start() {
    Log::get_logger()->info("{} started.", name_);
}

void ExecutionHandler::stop() {
    Log::get_logger()->info("{} stopped.", name_);
}

void ExecutionHandler::on_order(const Event& event) {
    const auto& order = static_cast<const OrderEvent&>(event);
    Log::get_logger()->info("{}: Received order to {} {} {}.", name_,
                             order.direction == OrderDirection::BUY ? "BUY" : "SELL", 
                             order.quantity, order.symbol);

    // In a real system, this is where you would interact with a brokerage API.
    // Here, we simulate the execution. For now, we assume the order is
    // filled instantly at a hardcoded price.
    // A more advanced simulation would model latency, slippage, and partial fills.
    double simulated_fill_price = 300.0; // Example price

    auto fill = std::make_shared<FillEvent>(
        order.symbol,
        order.direction,
        order.quantity,
        simulated_fill_price
    );

    event_bus_->publish(fill);
    Log::get_logger()->info("{}: Published fill event for {}.", name_, order.symbol);
}

} // namespace hft_system