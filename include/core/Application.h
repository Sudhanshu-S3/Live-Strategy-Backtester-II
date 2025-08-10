#ifndef HFT_SYSTEM_APPLICATION_H
#define HFT_SYSTEM_APPLICATION_H

#include "../config/Config.h"
#include <memory>
#include <map>
#include <string>
#include <thread>
#include <atomic>

namespace hft_system
{

    // Forward declarations...
    class EventBus;
    class DataHandler;
    class StrategyManager;
    class PortfolioManager;
    class RiskManager;
    class ExecutionHandler;
    class Analytics;
    class MLModelManager;

    class Application
    {
    public:
        Application(Config config);
        ~Application();

        void run();
        void stop();
        std::map<std::string, double> get_analytics_report();
        std::map<std::string, double> run_backtest();
        std::map<std::string, double> get_pnl_snapshot();

    private:
        void main_loop();

        Config config_;
        std::shared_ptr<EventBus> event_bus_;
        std::shared_ptr<DataHandler> data_handler_;
        std::shared_ptr<StrategyManager> strategy_manager_;
        std::shared_ptr<PortfolioManager> portfolio_manager_;
        std::shared_ptr<RiskManager> risk_manager_;
        std::shared_ptr<ExecutionHandler> execution_handler_;
        std::shared_ptr<Analytics> analytics_;
        std::shared_ptr<MLModelManager> ml_manager_; // Add this

        std::thread app_thread_;
        std::atomic<bool> is_running_{false};
    };

} // namespace hft_system
#endif // HFT_SYSTEM_APPLICATION_H