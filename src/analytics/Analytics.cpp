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

    void Analytics::start() { Log::get_logger()->info("{} started.", name_); }
    void Analytics::stop() { Log::get_logger()->info("{} stopped.", name_); }

    void Analytics::on_portfolio_update(const Event &event)
    {
        const auto &update = static_cast<const PortfolioUpdateEvent &>(event);
        equity_curve_.push_back(update.total_equity);
    }

    std::map<std::string, double> Analytics::generate_report(const std::list<Trade> &trade_log)
    {
        std::map<std::string, double> report;
        if (equity_curve_.size() < 2)
        {
            Log::get_logger()->warn("Not enough data to generate a performance report.");
            return report;
        }

        double initial_equity = equity_curve_.front();
        double final_equity = equity_curve_.back();

        report["initial_equity"] = initial_equity;
        report["final_equity"] = final_equity;
        report["total_return_pct"] = ((final_equity / initial_equity) - 1.0) * 100.0;

        // Max Drawdown
        double max_drawdown = 0.0, peak = equity_curve_[0];
        for (double equity : equity_curve_)
        {
            peak = std::max(peak, equity);
            max_drawdown = std::max(max_drawdown, (peak - equity) / peak);
        }
        report["max_drawdown_pct"] = max_drawdown * 100.0;

        // Sharpe & Sortino Ratios
        std::vector<double> returns;
        for (size_t i = 1; i < equity_curve_.size(); ++i)
        {
            returns.push_back((equity_curve_[i] / equity_curve_[i - 1]) - 1.0);
        }

        if (returns.size() > 1)
        {
            double sum = std::accumulate(returns.begin(), returns.end(), 0.0);
            double mean = sum / returns.size();
            double sq_sum = std::inner_product(returns.begin(), returns.end(), returns.begin(), 0.0);
            double stdev = std::sqrt(sq_sum / returns.size() - mean * mean);
            report["sharpe_ratio"] = (stdev > 1e-9) ? (mean / stdev) * std::sqrt(252) : 0.0;

            double downside_sq_sum = 0.0;
            for (double r : returns)
            {
                if (r < 0)
                    downside_sq_sum += r * r;
            }
            double downside_dev = std::sqrt(downside_sq_sum / returns.size());
            report["sortino_ratio"] = (downside_dev > 1e-9) ? (mean / downside_dev) * std::sqrt(252) : 0.0;
        }

        // Trade-Level Analysis
        if (!trade_log.empty())
        {
            int winning_trades = 0;
            double total_profit = 0.0, total_loss = 0.0;
            for (const auto &trade : trade_log)
            {
                if (trade.pnl > 0)
                {
                    winning_trades++;
                    total_profit += trade.pnl;
                }
                else
                {
                    total_loss += trade.pnl;
                }
            }
            report["total_trades"] = trade_log.size();
            report["win_rate_pct"] = (double)winning_trades / trade_log.size() * 100.0;
            report["profit_factor"] = (total_loss != 0) ? std::abs(total_profit / total_loss) : 0.0;
        }

        return report;
    }

} // namespace hft_system