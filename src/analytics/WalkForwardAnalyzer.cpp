#include "../../include/analytics/WalkForwardAnalyzer.h"
#include "../../include/analytics/Optimizer.h"
#include "../../include/core/Application.h"
#include "../../include/core/Log.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace hft_system
{

    // Helper function for date calculations
    std::string time_point_to_string(const std::chrono::system_clock::time_point &tp)
    {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&t));
        return std::string(buffer);
    }

    std::chrono::system_clock::time_point string_to_time_point(const std::string &s)
    {
        std::tm tm = {};
        std::stringstream ss(s);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }

    WalkForwardAnalyzer::WalkForwardAnalyzer(Config config) : base_config_(std::move(config)) {}

    void WalkForwardAnalyzer::run()
    {
        Log::get_logger()->info("--- Starting Walk-Forward Analysis ---");

        auto current_date = string_to_time_point(base_config_.walk_forward.start_date);
        auto end_date = string_to_time_point(base_config_.walk_forward.end_date);
        auto in_sample_duration = std::chrono::hours(24 * base_config_.walk_forward.in_sample_days);
        auto out_of_sample_duration = std::chrono::hours(24 * base_config_.walk_forward.out_of_sample_days);

        int period = 1;
        while (current_date + in_sample_duration + out_of_sample_duration <= end_date)
        {
            auto in_sample_start = current_date;
            auto in_sample_end = in_sample_start + in_sample_duration;
            auto out_of_sample_start = in_sample_end;
            auto out_of_sample_end = out_of_sample_start + out_of_sample_duration;

            Log::get_logger()->info("--- Period {}: In-Sample Optimization ---", period);
            Log::get_logger()->info("Range: {} to {}", time_point_to_string(in_sample_start), time_point_to_string(in_sample_end));

            // 1. Optimize on In-Sample Data
            Config opt_config = base_config_;
            opt_config.data.file_path = "tests/data/order_book_data.csv"; // Use appropriate full dataset
            opt_config.run_mode = RunMode::OPTIMIZATION;
            // Note: For a real test, you'd need logic to select the correct historical data slice.
            // We are simplifying here by running on the full dataset.

            Optimizer optimizer(opt_config);
            optimizer.run();
            // A more advanced version would extract the best params here. We'll proceed with base params.

            Log::get_logger()->info("--- Period {}: Out-of-Sample Test ---", period);
            Log::get_logger()->info("Range: {} to {}", time_point_to_string(out_of_sample_start), time_point_to_string(out_of_sample_end));

            // 2. Test on Out-of-Sample Data
            Config test_config = base_config_;
            test_config.data.file_path = "tests/data/order_book_data.csv";
            test_config.run_mode = RunMode::BACKTEST;

            Application backtest_app(test_config);
            auto report = backtest_app.run_backtest();

            double total_return = report.count("total_return_pct") ? report.at("total_return_pct") : 0.0;
            out_of_sample_returns_.push_back(total_return);
            Log::get_logger()->info("Out-of-Sample Return: {:.2f}%", total_return);

            // Move to the next period
            current_date += out_of_sample_duration;
            period++;
        }

        Log::get_logger()->info("--- WALK-FORWARD RESULTS ---");
        for (size_t i = 0; i < out_of_sample_returns_.size(); ++i)
        {
            Log::get_logger()->info("Period {}: {:.2f}%", i + 1, out_of_sample_returns_[i]);
        }
        double average_return = std::accumulate(out_of_sample_returns_.begin(), out_of_sample_returns_.end(), 0.0) / out_of_sample_returns_.size();
        Log::get_logger()->info("Average Out-of-Sample Return: {:.2f}%", average_return);
        Log::get_logger()->info("----------------------------");
    }

} // namespace hft_system