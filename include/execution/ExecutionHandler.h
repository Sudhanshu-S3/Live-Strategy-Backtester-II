#ifndef HFT_SYSTEM_EXECUTIONHANDLER_H
#define HFT_SYSTEM_EXECUTIONHANDLER_H

#include "../core/Component.h"
#include "../events/Event.h"
#include "../config/Config.h"
namespace hft_system
{

    class ExecutionHandler : public Component
    {
    public:
        // Constructor DECLARATION
        ExecutionHandler(std::shared_ptr<EventBus> event_bus, std::string name, const ExecutionConfig &config);

        // Method DECLARATIONS
        void start() override;
        void stop() override;

    private:
        // Event handler DECLARATION
        void on_order(const Event &event);
        ExecutionConfig config_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_EXECUTIONHANDLER_H