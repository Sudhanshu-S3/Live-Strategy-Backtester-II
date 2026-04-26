#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <future>
#include <memory>
#include <filesystem>
#include <pthread.h>
#include <sched.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <cmath>

#include "core/Log.h"
#include "core/EventBus.h"
#include "core/Application.h"
#include "config/ConfigParser.h"
#include "config/Config.h"
#include "utils/Timer.h"
#include "utils/PerformanceMonitor.h"

using namespace hft_system;

class PerformanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init();
    }

    void TearDown() override
    {
    }
};

static void pin_to_core(int core_id) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

TEST_F(PerformanceTest, EndToEndSystemPerformance)
{
    pin_to_core(2);   // pick an isolated core if you have isolcpus, else any non-0 core

    // Configure the application to use the test data
    Config config;
    config.run_mode = RunMode::BACKTEST;
    config.initial_capital = 100000.0;
    config.data.symbol = "TEST_BTC";
    config.data.file_path = std::string(PROJECT_SOURCE_DIR) + "/tests/data/test_market_data.csv";
    config.execution.commission_pct = 0.001;
    config.execution.slippage_pct = 0.0005;
    config.risk.risk_per_trade_pct = 0.01;
    config.risk.use_dynamic_sizing = true;

    // Add a simple strategy
    StrategyConfig strategy;
    strategy.name = "ORDER_BOOK_IMBALANCE";
    strategy.symbol = "TEST_BTC";
    strategy.params.lookback_levels = 10;
    strategy.params.imbalance_threshold = 1.2;
    config.strategies.push_back(strategy);

    constexpr int kWarmup = 3;
    constexpr int kIterations = 20;
    std::vector<int64_t> times_ns;
    times_ns.reserve(kIterations);

    for (int i = 0; i < kWarmup + kIterations; ++i) {
        PerformanceMonitor::get_instance().reset();
        Timer t("Full_System_Test");
        
        Application app(config);
        app.run(); // Calls the full initialization and backtest processing
        
        int64_t ns = t.elapsed_nanoseconds();
        if (i >= kWarmup) times_ns.push_back(ns);
    }

    std::sort(times_ns.begin(), times_ns.end());
    int64_t median = times_ns[times_ns.size() / 2];
    int64_t p95    = times_ns[static_cast<size_t>(times_ns.size() * 0.95)];
    int64_t min    = times_ns.front();
    int64_t max    = times_ns.back();
    double  mean   = std::accumulate(times_ns.begin(), times_ns.end(), 0.0) / times_ns.size();
    double  var    = 0.0;
    for (auto v : times_ns) var += (v - mean) * (v - mean);
    double stddev  = std::sqrt(var / times_ns.size());

    Log::get_logger()->info("Iterations={}, warmup={}, median={} ns, p95={} ns, min={} ns, max={} ns, stddev={:.0f} ns",
                            times_ns.size(), kWarmup, median, p95, min, max, stddev);

    // --- Dump Per-Stage Latency Table ---
    Log::get_logger()->info("-----------------------------------------------------------------------------------------");
    Log::get_logger()->info("{:<40s} | {:>8s} | {:>8s} | {:>8s} | {:>8s}", "Stage", "Count", "p50 (ns)", "p95 (ns)", "p99 (ns)");
    Log::get_logger()->info("-----------------------------------------------------------------------------------------");

    auto metrics = PerformanceMonitor::get_instance().get_statistics();
    for (const auto &[name, stats] : metrics) {
        // Only print if the statistics we need are present
        if (stats.count("count") && stats.count("p50_ns") && stats.count("p95_ns") && stats.count("p99_ns")) {
            Log::get_logger()->info("{:<40s} | {:>8.0f} | {:>8.0f} | {:>8.0f} | {:>8.0f}",
                name,
                stats.at("count"),
                stats.at("p50_ns"),
                stats.at("p95_ns"),
                stats.at("p99_ns"));
        }
    }
    Log::get_logger()->info("-----------------------------------------------------------------------------------------");

    ASSERT_GT(times_ns.size(), 0u);
}