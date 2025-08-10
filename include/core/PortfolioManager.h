#ifndef HFT_SYSTEM_PORTFOLIOMANAGER_H
#define HFT_SYSTEM_PORTFOLIOMANAGER_H

#include "Component.h"
#include "../events/Event.h"
#include "DataTypes.h"
#include <map>
#include <list>

namespace hft_system {

// A struct to hold the state of a current open position.
struct Position {
    std::string symbol;
    OrderDirection direction;
    int quantity = 0;
    double entry_price = 0.0;
};

class PortfolioManager : public Component {
public:
    PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital);
    
    void start() override;
    void stop() override;

    // Public getter for the analytics component to access the trade log.
    const std::list<Trade>& get_trade_log() const { return trade_log_; }

private:
    void on_fill(const Event& event);

    double capital_;
    double cash_;
    std::map<std::string, Position> positions_;
    std::list<Trade> trade_log_;
};

} // namespace hft_system
#endif // HFT_SYSTEM_PORTFOLIOMANAGER_H