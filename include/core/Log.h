// include/core/Log.h
#ifndef HFT_SYSTEM_LOG_H
#define HFT_SYSTEM_LOG_H

#include "spdlog/spdlog.h"
#include <memory>

namespace hft_system
{

    class Log
    {
    public:
        static void init();
        static void shutdown();
        static std::shared_ptr<spdlog::logger> &get_logger()
        {
            return logger_;
        }

    private:
        static std::shared_ptr<spdlog::logger> logger_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_LOG_H