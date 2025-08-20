#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <memory>
#include <numeric>

namespace hft_system
{
    // Forward declare Timer class
    class Timer;

    class PerformanceMonitor
    {
    public:
        static PerformanceMonitor &get_instance()
        {
            static PerformanceMonitor instance;
            return instance;
        }

        void record_metric(const std::string &name, int64_t nanoseconds)
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_[name].push_back(nanoseconds);
        }

        std::map<std::string, std::map<std::string, double>> get_statistics() const
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            std::map<std::string, std::map<std::string, double>> results;

            for (const auto &[name, measurements] : metrics_)
            {
                if (measurements.empty())
                    continue;

                auto &stats = results[name];

                // Calculate min/max/avg
                int64_t min_time = *std::min_element(measurements.begin(), measurements.end());
                int64_t max_time = *std::max_element(measurements.begin(), measurements.end());
                int64_t total_time = std::accumulate(measurements.begin(), measurements.end(), int64_t(0));
                double avg_time = static_cast<double>(total_time) / measurements.size();

                stats["min_ns"] = min_time;
                stats["max_ns"] = max_time;
                stats["avg_ns"] = avg_time;
                stats["count"] = measurements.size();

                // Calculate percentiles (90th, 95th, 99th)
                if (measurements.size() > 1)
                {
                    std::vector<int64_t> sorted_measurements = measurements;
                    std::sort(sorted_measurements.begin(), sorted_measurements.end());

                    size_t p90_idx = static_cast<size_t>(measurements.size() * 0.9);
                    size_t p95_idx = static_cast<size_t>(measurements.size() * 0.95);
                    size_t p99_idx = static_cast<size_t>(measurements.size() * 0.99);

                    stats["p90_ns"] = sorted_measurements[p90_idx];
                    stats["p95_ns"] = sorted_measurements[p95_idx];
                    stats["p99_ns"] = sorted_measurements[p99_idx];
                }
            }

            return results;
        }

        void reset()
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.clear();
        }

    private:
        PerformanceMonitor() = default;
        ~PerformanceMonitor() = default;

        mutable std::mutex metrics_mutex_;
        std::map<std::string, std::vector<int64_t>> metrics_;
    };

    // TimerGuard class to automatically record elapsed time when it goes out of scope
    class TimerGuard
    {
    public:
        TimerGuard(const std::string &function_name, Timer &t, PerformanceMonitor &pm);
        ~TimerGuard();

    private:
        std::string function_name_;
        Timer &timer_;
        PerformanceMonitor &perf_monitor_;
    };

} // namespace hft_system

// Helper macro for timing functions and recording metrics
#define TIME_FUNCTION(name_str) hft_system::Timer timer(name_str); auto &perf_monitor = hft_system::PerformanceMonitor::get_instance(); hft_system::TimerGuard timer_guard(name_str, timer, perf_monitor)