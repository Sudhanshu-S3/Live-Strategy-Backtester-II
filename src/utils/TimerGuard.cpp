#include "../../include/utils/PerformanceMonitor.h"
#include "../../include/utils/Timer.h"

namespace hft_system
{

    TimerGuard::TimerGuard(const std::string &function_name, Timer &t, PerformanceMonitor &pm)
        : function_name_(function_name), timer_(t), perf_monitor_(pm)
    {
    }

    TimerGuard::~TimerGuard()
    {
        perf_monitor_.record_metric(function_name_, timer_.elapsed_nanoseconds());
    }

} // namespace hft_system