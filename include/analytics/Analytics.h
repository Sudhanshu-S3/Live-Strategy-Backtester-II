#ifndef HFT_SYSTEM_ANALYTICS_H
#define HFT_SYSTEM_ANALYTICS_H

#include "../core/Component.h"
#include "../config/Config.h"
#include <vector>

namespace hft_system
{

    class Analytics : public Component
    {
    public:
        Analytics(std::shared_ptr<EventBus> event_bus, std::string name, const AnalyticsConfig &config);

        void start() override;
        void stop() override;

        void generate_report();

    private:
        void on_portfolio_update(const Event &event);

        AnalyticsConfig config_;
        std::vector<double> equity_curve_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_ANALYTICS_H