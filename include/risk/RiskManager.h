
#ifndef HFT_SYSTEM_RISKMANAGER_H
#define HFT_SYSTEM_RISKMANAGER_H

#include "../core/Component.h"
#include "../config/Config.h"
#include <map>

namespace hft_system
{

    class RiskManager : public Component
    {
    public:
        RiskManager(std::shared_ptr<EventBus> event_bus, std::string name, const Config &config);

        void start() override;
        void stop() override;

    private:
        void on_signal(const Event &event);
        void on_market(const Event &event);
        void on_portfolio_update(const Event &event);

        RiskConfig risk_config_;

        // Cached state
        double latest_equity_ = 0.0;
        double latest_cash_ = 0.0;
        std::map<std::string, double> latest_prices_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_RISKMANAGER_H