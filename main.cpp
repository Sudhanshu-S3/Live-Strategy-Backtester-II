#include "include/core/Log.h"
#include "include/config/ConfigParser.h"
#include "include/analytics/Optimizer.h"
#include "include/analytics/WalkForwardAnalyzer.h" // Include this
#include "include/core/Application.h"

int main(int argc, char** argv) {
    hft_system::Log::init();
    try {
        hft_system::Config config = hft_system::ConfigParser::parse("config.json");

        if (config.run_mode == hft_system::RunMode::OPTIMIZATION) {
            hft_system::Optimizer optimizer(config);
            optimizer.run();
            optimizer.print_results();
        } else if (config.run_mode == hft_system::RunMode::WALK_FORWARD) { // Add this block
            hft_system::WalkForwardAnalyzer wf_analyzer(config);
            wf_analyzer.run();
        } else {
            hft_system::Application app(config);
            if (config.run_mode == hft_system::RunMode::BACKTEST) {
                app.run_backtest();
            } else {
                // Live mode would need to be re-integrated into the Application class
            }
        }

    } catch (const std::exception& e) {
        hft_system::Log::get_logger()->critical("Fatal Application Error: {}", e.what());
    }
    hft_system::Log::shutdown();
    return 0;
}