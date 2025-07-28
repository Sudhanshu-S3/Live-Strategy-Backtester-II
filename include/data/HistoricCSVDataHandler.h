// src/data/HistoricCSVDataHandler.h
#ifndef HFT_SYSTEM_HISTORICCSVDATAHANDLER_H
#define HFT_SYSTEM_HISTORICCSVDATAHANDLER_H

#include "../../include/data/DataHandler.h"
#include <string>
#include <thread>
#include <atomic>

namespace hft_system {

class HistoricCSVDataHandler : public DataHandler {
public:
    HistoricCSVDataHandler(std::shared_ptr<EventBus> event_bus, std::string symbol, std::string file_path);
    ~HistoricCSVDataHandler();

    // Overrides from the Component base class
    void start() override;
    void stop() override;

    // Override from the DataHandler base class
    void run() override;

private:
    std::string symbol_;
    std::string file_path_;
    std::thread data_thread_;
    std::atomic<bool> is_running_;
};

} // namespace hft_system

#endif // HFT_SYSTEM_HISTORICCSVDATAHANDLER_H