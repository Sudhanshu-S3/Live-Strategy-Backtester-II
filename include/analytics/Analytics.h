/**
 * @file Analytics.h
 * @brief Public API declarations for Analytics.
 */

#ifndef HFT_SYSTEM_ANALYTICS_H
#define HFT_SYSTEM_ANALYTICS_H

#include "../core/Component.h"
#include "../config/Config.h"
#include "../core/DataTypes.h"
#include <vector>
#include <list>
#include <map>
#include <string>

namespace hft_system
{

    class Analytics : public Component
    {
    public:
        Analytics(std::shared_ptr<EventBus> event_bus, std::string name, const AnalyticsConfig &config);

        void start() override;
        void stop() override;

        // This now returns a map of key-value metrics.
/** @brief generate report. */
        std::map<std::string, double> generate_report(const std::list<Trade> &trade_log);

    private:
/** @brief on portfolio update. */
        void on_portfolio_update(const Event &event);
        double bars_per_year_ = 0.0;
        AnalyticsConfig config_;
        std::vector<double> equity_curve_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_ANALYTICS_H