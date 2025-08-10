#ifndef HFT_SYSTEM_MONTECARLOSIMULATOR_H
#define HFT_SYSTEM_MONTECARLOSIMULATOR_H

#include "../core/DataTypes.h"
#include <vector>
#include <list>
#include <random>

namespace hft_system
{

    class MonteCarloSimulator
    {
    public:
        MonteCarloSimulator(const std::list<Trade> &trade_log, int num_simulations, double initial_equity);

        void run();
        void print_results() const;

    private:
        std::vector<double> trade_returns_;
        int num_simulations_;
        double initial_equity_;
        std::vector<std::vector<double>> simulation_equity_curves_;
        std::mt19937 random_generator_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_MONTECARLOSIMULATOR_H