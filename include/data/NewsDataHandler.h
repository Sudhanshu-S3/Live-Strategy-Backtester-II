#ifndef HFT_SYSTEM_NEWSDATAHANDLER_H
#define HFT_SYSTEM_NEWSDATAHANDLER_H

#include "DataHandler.h"

namespace hft_system
{

    class NewsDataHandler : public DataHandler
    {
    public:
        NewsDataHandler(std::shared_ptr<EventBus> event_bus, std::string name);

        void start() override;
        void stop() override;
        void run() override;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_NEWSDATAHANDLER_H