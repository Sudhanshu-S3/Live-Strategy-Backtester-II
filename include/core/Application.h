// include/core/Application.h
#ifndef HFT_SYSTEM_APPLICATION_H
#define HFT_SYSTEM_APPLICATION_H

#include "EventBus.h"
#include "../config/Config.h"
#include <memory>
#include <thread>

namespace hft_system
{

    // Forward declarations of our components
    class DataHandler;
    class StrategyManager;
    class PortfolioManager;
    class RiskManager;
    class ExecutionHandler;
    class Analytics;

    class Application
    {
    public:
        Application();
        ~Application();

        void run();
        void stop();
        std::map<std::string, double> get_analytics_report();

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

        std::thread app_thread_;
        std::atomic<bool> is_running_{false};
    };

} // namespace hft_system

#endif // HFT_SYSTEM_APPLICATION_H