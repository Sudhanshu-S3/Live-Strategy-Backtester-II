#ifndef HFT_SYSTEM_PORTFOLIOMANAGER_H
#define HFT_SYSTEM_PORTFOLIOMANAGER_H

#include "Component.h"
#include "../events/Event.h"
#include "DataTypes.h"
#include <map>
#include <list>

namespace hft_system
{

    struct Position
    {
        std::string symbol;
        OrderDirection direction;
        double quantity = 0.0; // Changed from int to double
        double entry_price = 0.0;
    };

    class PortfolioManager : public Component
    {
    public:
        PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital);

        void start() override;
        void stop() override;

        const std::list<Trade> &get_trade_log() const;
        std::map<std::string, double> get_pnl_summary() const;

    private:
        void on_fill(const Event &event);

        double capital_;
        double cash_;
        std::map<std::string, Position> positions_;
        std::list<Trade> trade_log_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_PORTFOLIOMANAGER_H