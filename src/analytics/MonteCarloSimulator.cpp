#include "../../include/analytics/MonteCarloSimulator.h"
#include "../../include/core/Log.h"
#include <algorithm>
#include <iostream>

namespace hft_system
{

    MonteCarloSimulator::MonteCarloSimulator(const std::list<Trade> &trade_log, int num_simulations, double initial_equity)
        : num_simulations_(num_simulations), initial_equity_(initial_equity)
    {

        // Seed the random number generator
        std::random_device rd;
        random_generator_.seed(rd());

        // Convert P&L of each trade into a percentage return based on initial equity
        for (const auto &trade : trade_log)
        {
            trade_returns_.push_back(trade.pnl / initial_equity_);
        }
    }

    void MonteCarloSimulator::run()
    {
        if (trade_returns_.empty())
        {
            Log::get_logger()->warn("Monte Carlo: No trades to simulate. Skipping.");
            return;
        }

        Log::get_logger()->info("Running Monte Carlo simulation with {} paths...", num_simulations_);
        simulation_equity_curves_.resize(num_simulations_);

        std::uniform_int_distribution<int> dist(0, trade_returns_.size() - 1);

        for (int i = 0; i < num_simulations_; ++i)
        {
            double current_equity = initial_equity_;
            simulation_equity_curves_[i].push_back(current_equity);

            for (size_t j = 0; j < trade_returns_.size(); ++j)
            {
                // Randomly sample one trade from the historical results
                int random_index = dist(random_generator_);
                double simulated_return = trade_returns_[random_index];

                // Apply the return to the current equity path
                current_equity *= (1.0 + simulated_return);
                simulation_equity_curves_[i].push_back(current_equity);
            }
        }
        Log::get_logger()->info("Monte Carlo simulation complete.");
    }

    void MonteCarloSimulator::print_results() const
    {
        if (simulation_equity_curves_.empty())
        {
            return;
        }

        Log::get_logger()->info("--- MONTE CARLO RESULTS ---");

        std::vector<double> final_equities;
        std::vector<double> max_drawdowns;

        for (const auto &path : simulation_equity_curves_)
        {
            final_equities.push_back(path.back());

            double peak = path[0];
            double max_dd = 0.0;
            for (double equity : path)
            {
                peak = std::max(peak, equity);
                max_dd = std::max(max_dd, (peak - equity) / peak);
            }
            max_drawdowns.push_back(max_dd);
        }

        std::sort(final_equities.begin(), final_equities.end());
        std::sort(max_drawdowns.begin(), max_drawdowns.end());

        double average_final_equity = std::accumulate(final_equities.begin(), final_equities.end(), 0.0) / final_equities.size();
        double percentile_5 = final_equities[static_cast<size_t>(num_simulations_ * 0.05)];
        double percentile_95 = final_equities[static_cast<size_t>(num_simulations_ * 0.95)];
        double average_max_dd = std::accumulate(max_drawdowns.begin(), max_drawdowns.end(), 0.0) / max_drawdowns.size();

        Log::get_logger()->info("Average Final Equity: ${:.2f}", average_final_equity);
        Log::get_logger()->info("5th Percentile Equity:  ${:.2f}", percentile_5);
        Log::get_logger()->info("95th Percentile Equity: ${:.2f}", percentile_95);
        Log::get_logger()->info("Average Max Drawdown:   {:.2f}%", average_max_dd * 100.0);
        Log::get_logger()->info("---------------------------");
    }

} // namespace hft_system