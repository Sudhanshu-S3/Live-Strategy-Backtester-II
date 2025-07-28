// include/data/DataHandler.h
#ifndef HFT_SYSTEM_DATAHANDLER_H
#define HFT_SYSTEM_DATAHANDLER_H

#include "../core/Component.h"

namespace hft_system {

// Abstract base class for all data handlers (live or historical).
// Its sole responsibility is to fetch data and publish MarketEvents.
class DataHandler : public Component {
public:
    DataHandler(std::shared_ptr<EventBus> event_bus, std::string name)
        : Component(event_bus, std::move(name)) {}

    // Main loop for the data handler's thread.
    // For historical data, it reads from a file.
    // For live data, it listens to a websocket.
    virtual void run() = 0;
};

} // namespace hft_system

#endif // HFT_SYSTEM_DATAHANDLER_H