#ifndef HFT_SYSTEM_CONFIG_H
#define HFT_SYSTEM_CONFIG_H

#include <string>
#include <vector>

namespace hft_system
{

    // Corresponds to the "execution" object in the JSON
    struct ExecutionConfig
    {
        double commission_pct = 0.001; // 0.1% commission
        double slippage_pct = 0.0005;  // 0.05% slippage
    };

    // Corresponds to the "data" object in the JSON
    struct DataConfig
    {
        std::string symbol;
        std::string file_path;
    };
    struct RiskConfig
    {
        double risk_per_trade_pct = 0.01; // 1% of total equity per trade
    };

    // Main configuration struct
    struct Config
    {
        double initial_capital = 100000.0;
        DataConfig data;
        ExecutionConfig execution;
        RiskConfig risk; // ADD THIS LINE
    };

} // namespace hft_system

#endif // HFT_SYSTEM_CONFIG_H