#include "../../include/core/Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace hft_system
{

    std::shared_ptr<spdlog::logger> Log::logger_;

    void Log::init()
    {
        // Log to console with colors
        logger_ = spdlog::stdout_color_mt("console");
        spdlog::set_level(spdlog::level::trace); // Log all levels, from detailed to critical
        logger_->info("Logging initialized.");
    }
    void Log::shutdown()
    {
        spdlog::shutdown();
        logger_ = nullptr;
    }

} // namespace hft_system