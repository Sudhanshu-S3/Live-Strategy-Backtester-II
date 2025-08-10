#ifndef HFT_SYSTEM_APPLICATION_H
#define HFT_SYSTEM_APPLICATION_H

#include "../config/Config.h"
#include <memory>
#include <map>
#include <string>

// Forward declarations to keep this header clean
namespace hft_system
{
    class EventBus;
    class DataHandler;
    class StrategyManager;
    class PortfolioManager;
    class RiskManager;
    class ExecutionHandler;
    class Analytics;

    class Application
    {
    public:
        // The constructor now takes a Config object, making it testable
        Application(Config config);

        // This is the main entry point for running a single backtest
        std::map<std::string, double> run_backtest();

    private:
        Config config_;
        std::shared_ptr<EventBus> event_bus_;
        std::shared_ptr<DataHandler> data_handler_;
        std::shared_ptr<StrategyManager> strategy_manager_;
        std::shared_ptr<PortfolioManager> portfolio_manager_;
        std::shared_ptr<RiskManager> risk_manager_;
        std::shared_ptr<ExecutionHandler> execution_handler_;
        std::shared_ptr<Analytics> analytics_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_APPLICATION_H