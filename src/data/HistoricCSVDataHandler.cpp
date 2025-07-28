// src/data/HistoricCSVDataHandler.cpp
#include "../../include/data/HistoricCSVDataHandler.h"
#include "../../include/events/Event.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include "../../include/core/Log.h"

namespace hft_system {

HistoricCSVDataHandler::HistoricCSVDataHandler(std::shared_ptr<EventBus> event_bus, std::string symbol, std::string file_path)
    : DataHandler(event_bus, "HistoricCSVDataHandler"),
      symbol_(std::move(symbol)),
      file_path_(std::move(file_path)),
      is_running_(false) {}

HistoricCSVDataHandler::~HistoricCSVDataHandler() {
    if (is_running_.load()) {
        stop();
    }
}

void HistoricCSVDataHandler::start() {
    is_running_.store(true);
    data_thread_ = std::thread(&HistoricCSVDataHandler::run, this);
}

void HistoricCSVDataHandler::stop() {
    is_running_.store(false);
    if (data_thread_.joinable()) {
        data_thread_.join();
    }
}

void HistoricCSVDataHandler::run() {
    Log::get_logger()->info("DataHandler thread started for symbol {} from file {}.", symbol_, file_path_);

    std::ifstream file(file_path_);
    if (!file.is_open()) {
        Log::get_logger()->error("FATAL: Could not open data file: {}", file_path_);
        return;
    }
    Log::get_logger()->info("Successfully opened data file: {}", file_path_);

    std::string line;
    // Skip the header line
    std::getline(file, line);

    while (is_running_.load() && std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;
        std::string timestamp_str;
        
        // **This is the missing declaration**
        std::string price_str;

        // Assuming CSV format: timestamp,open,high,low,close,volume
        std::getline(ss, timestamp_str, ','); // Timestamp
        std::getline(ss, item, ',');         // Open
        std::getline(ss, item, ',');         // High
        std::getline(ss, item, ',');         // Low
        std::getline(ss, price_str, ',');    // Close

        try {
            double price = std::stod(price_str);
            auto market_event = std::make_shared<MarketEvent>(symbol_, price);
            Log::get_logger()->trace("Publishing MarketEvent, Price: {}", price);
            event_bus_->publish(market_event);
        } catch (const std::invalid_argument& e) {
            // Skip malformed lines
        }

        // Simulate a live data feed with a small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    Log::get_logger()->info("DataHandler finished processing file: {}.", file_path_);
}
} // namespace hft_system