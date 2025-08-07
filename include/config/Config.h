#ifndef HFT_SYSTEM_CONFIG_H
#define HFT_SYSTEM_CONFIG_H

#include <string>
#include <vector>

namespace hft_system
{

    enum class RunMode
    {
        BACKTEST,
        LIVE
    };

    struct DataConfig
    {
        std::string symbol;
        std::string file_path;
    };

    struct ExecutionConfig
    {
        double commission_pct = 0.001;
        double slippage_pct = 0.0005;
    };

    struct RiskConfig
    {
        double risk_per_trade_pct = 0.01;
    };

    struct AnalyticsConfig
    {
        bool calculate_sharpe = true;
        bool calculate_max_drawdown = true;
    };

    struct StrategyParams
    {
        int lookback_levels = 10;
        double imbalance_threshold = 1.5;
    };

    struct StrategyConfig
    {
        std::string name;
        std::string symbol;
        StrategyParams params;
    };

    struct WebSocketConfig
    {
        std::string host;
        int port;
        std::string target;
    };

    struct Config
    {
        RunMode run_mode = RunMode::BACKTEST;
        double initial_capital = 100000.0;
        DataConfig data;
        ExecutionConfig execution;
        RiskConfig risk;
        AnalyticsConfig analytics;
        std::vector<StrategyConfig> strategies;
        WebSocketConfig websocket;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_CONFIG_H