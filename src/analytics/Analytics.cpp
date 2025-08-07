#include "../../include/analytics/Analytics.h"
#include "../../include/core/Log.h"
#include <functional>
#include <numeric>
#include <cmath>
#include <algorithm>

namespace hft_system
{

    Analytics::Analytics(std::shared_ptr<EventBus> event_bus, std::string name, const AnalyticsConfig &config)
        : Component(event_bus, std::move(name)), config_(config)
    {

        using namespace std::placeholders;
        event_bus_->subscribe(EventType::PORTFOLIO_UPDATE, std::bind(&Analytics::on_portfolio_update, this, _1));
    }

    void Analytics::start()
    {
        Log::get_logger()->info("{} started.", name_);
    }

    void Analytics::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void Analytics::on_portfolio_update(const Event &event)
    {
        const auto &update = static_cast<const PortfolioUpdateEvent &>(event);
        equity_curve_.push_back(update.total_equity);
    }

    void Analytics::generate_report()
    {
        if (equity_curve_.size() < 2)
        {
            Log::get_logger()->warn("Not enough data to generate a performance report.");
            return;
        }

        Log::get_logger()->info("--- PERFORMANCE REPORT ---");

        double initial_equity = equity_curve_.front();
        double final_equity = equity_curve_.back();
        double total_return_pct = ((final_equity / initial_equity) - 1.0) * 100.0;

        Log::get_logger()->info("Initial Equity: ${:.2f}", initial_equity);
        Log::get_logger()->info("Final Equity:   ${:.2f}", final_equity);
        Log::get_logger()->info("Total Return:   {:.2f}%", total_return_pct);

        if (config_.calculate_max_drawdown)
        {
            double max_drawdown = 0.0;
            double peak = equity_curve_[0];
            for (double equity : equity_curve_)
            {
                if (equity > peak)
                {
                    peak = equity;
                }
                double drawdown = (peak - equity) / peak;
                if (drawdown > max_drawdown)
                {
                    max_drawdown = drawdown;
                }
            }
            Log::get_logger()->info("Max Drawdown:   {:.2f}%", max_drawdown * 100.0);
        }

        Log::get_logger()->info("--------------------------");
    }

} // namespace hft_system