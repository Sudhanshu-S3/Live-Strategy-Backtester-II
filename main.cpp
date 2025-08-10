#include "include/core/Log.h"
#include "include/config/ConfigParser.h"
#include "include/core/Application.h"
#include "include/analytics/Optimizer.h"
#include "include/analytics/WalkForwardAnalyzer.h"
#include <iostream>

int main(int argc, char **argv)
{
    hft_system::Log::init();
    try
    {
        hft_system::Config config = hft_system::ConfigParser::parse("config.json");

        switch (config.run_mode)
        {
        case hft_system::RunMode::OPTIMIZATION:
        {
            hft_system::Optimizer optimizer(config);
            optimizer.run();
            optimizer.print_results();
            break;
        }
        case hft_system::RunMode::WALK_FORWARD:
        {
            hft_system::WalkForwardAnalyzer wf_analyzer(config);
            wf_analyzer.run();
            break;
        }
        case hft_system::RunMode::BACKTEST:
        {
            hft_system::Application app(config);
            app.run_backtest();
            break;
        }
        case hft_system::RunMode::LIVE:
        {
            hft_system::Log::get_logger()->info("Live mode not fully implemented in this launcher. Please use API server.");
            // In a future step, the Application class would handle live mode.
            break;
        }
        }
    }
    catch (const std::exception &e)
    {
        hft_system::Log::get_logger()->critical("Fatal Application Error: {}", e.what());
    }
    hft_system::Log::shutdown();
    return 0;
}