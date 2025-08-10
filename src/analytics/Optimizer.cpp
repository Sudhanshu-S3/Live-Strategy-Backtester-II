#include "../../include/analytics/Optimizer.h"
#include "../../include/core/Application.h" // We will run the Application class
#include "../../include/core/Log.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace hft_system {

Optimizer::Optimizer(Config config) : base_config_(std::move(config)) {
    generate_test_configs();
}

void Optimizer::generate_test_configs() {
    // This is a simple grid search for one parameter. A more advanced
    // version would handle multiple parameters recursively.
    const auto& ranges = base_config_.optimization.param_ranges;
    if (ranges.find("imbalance_threshold") != ranges.end()) {
        for (double threshold : ranges.at("imbalance_threshold")) {
            Config new_config = base_config_;
            // Find the strategy and update its parameter
            for (auto& strat : new_config.strategies) {
                if (strat.name == base_config_.optimization.strategy_name) {
                    strat.params.imbalance_threshold = threshold;
                }
            }
            test_configs_.push_back(new_config);
        }
    }
}

void Optimizer::run() {
    Log::get_logger()->info("--- Starting Strategy Optimization ---");
    Log::get_logger()->info("Running {} backtests...", test_configs_.size());

    for (const auto& config : test_configs_) {
        // Create a temporary Application instance to run a single backtest
        Application backtest_app(config);
        auto report = backtest_app.run_backtest();

        // Store the result (Sharpe Ratio)
        double sharpe = report.count("sharpe_ratio") ? report.at("sharpe_ratio") : 0.0;
        
        // Create a string key to represent the parameter set
        std::stringstream ss;
        ss << "threshold:" << config.strategies[0].params.imbalance_threshold;
        results_[ss.str()] = sharpe;

        Log::get_logger()->info("Params: {} -> Sharpe Ratio: {:.2f}", ss.str(), sharpe);
    }
}

void Optimizer::print_results() const {
    Log::get_logger()->info("--- OPTIMIZATION RESULTS ---");
    if (results_.empty()) {
        Log::get_logger()->warn("No results to display.");
        return;
    }

    // Find the best parameter set
    auto best_result = std::max_element(results_.begin(), results_.end(), 
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

    Log::get_logger()->info("Best Parameter Set Found:");
    Log::get_logger()->info("  Params: {}", best_result->first);
    Log::get_logger()->info("  Sharpe Ratio: {:.2f}", best_result->second);
    Log::get_logger()->info("--------------------------");
}

} // namespace hft_system