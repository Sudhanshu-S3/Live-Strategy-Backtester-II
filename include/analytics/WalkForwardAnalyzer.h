/**
 * @file WalkForwardAnalyzer.h
 * @brief Public API declarations for WalkForwardAnalyzer.
 */

#ifndef HFT_SYSTEM_WALKFORWARDANALYZER_H
#define HFT_SYSTEM_WALKFORWARDANALYZER_H

#include "../config/Config.h"
#include <vector>
#include <string>

namespace hft_system
{

    class WalkForwardAnalyzer
    {
    public:
        WalkForwardAnalyzer(Config config);
/** @brief run. */
        void run();

    private:
        Config base_config_;
        std::vector<double> out_of_sample_returns_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_WALKFORWARDANALYZER_H