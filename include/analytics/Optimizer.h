#ifndef HFT_SYSTEM_OPTIMIZER_H
#define HFT_SYSTEM_OPTIMIZER_H

#include "../config/Config.h"
#include <map>
#include <string>
#include <vector>

namespace hft_system {

class Optimizer {
public:
    Optimizer(Config config);

    void run();
    void print_results() const;

private:
    Config base_config_;
    std::vector<Config> test_configs_;
    std::map<std::string, double> results_; // Stores results as {param_set_string -> sharpe_ratio}

    void generate_test_configs();
};

} // namespace hft_system

#endif // HFT_SYSTEM_OPTIMIZER_H