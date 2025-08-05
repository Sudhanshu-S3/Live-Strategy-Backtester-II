#ifndef HFT_SYSTEM_CONFIG_H
#define HFT_SYSTEM_CONFIG_H

#include <string>
#include <vector>

namespace hft_system
{

    // Corresponds to the "data" object in the JSON
    struct DataConfig
    {
        std::string symbol;
        std::string file_path;
    };

    // Main configuration struct
    struct Config
    {
        double initial_capital = 100000.0;
        DataConfig data;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_CONFIG_H