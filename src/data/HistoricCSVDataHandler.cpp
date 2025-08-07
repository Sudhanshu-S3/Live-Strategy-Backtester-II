#include "../../include/data/HistoricCSVDataHandler.h"
#include "../../include/events/Event.h"
#include "simdjson.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include "../../include/core/Log.h"

namespace hft_system
{

    HistoricCSVDataHandler::HistoricCSVDataHandler(std::shared_ptr<EventBus> event_bus, std::string symbol, std::string file_path)
        : DataHandler(event_bus, "HistoricCSVDataHandler"),
          symbol_(std::move(symbol)),
          file_path_(std::move(file_path)),
          is_running_(false) {}

    HistoricCSVDataHandler::~HistoricCSVDataHandler()
    {
        if (is_running_.load())
        {
            stop();
        }
    }

    void HistoricCSVDataHandler::start()
    {
        is_running_.store(true);
        data_thread_ = std::thread(&HistoricCSVDataHandler::run, this);
    }

    void HistoricCSVDataHandler::stop()
    {
        is_running_.store(false);
        if (data_thread_.joinable())
        {
            data_thread_.join();
        }
    }

    void HistoricCSVDataHandler::run()
    {
        Log::get_logger()->info("DataHandler thread started for symbol {} from file {}.", symbol_, file_path_);
        std::ifstream file(file_path_);
        if (!file.is_open())
        {
            Log::get_logger()->error("FATAL: Could not open data file: {}", file_path_);
            return;
        }

        simdjson::ondemand::parser parser;
        std::string line;
        std::getline(file, line); // Skip header

        while (is_running_.load() && std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string ts_str, bids_str, asks_str;

            std::getline(ss, ts_str, ',');

            // Handle the quoted strings for bids and asks
            char quote;
            ss >> quote;                     // Consume the opening "
            std::getline(ss, bids_str, '"'); // Read until the closing "
            ss >> quote;                     // Consume the trailing comma
            ss >> quote;                     // Consume the opening " for asks
            std::getline(ss, asks_str, '"'); // Read until the closing " for asks

            try
            {
                OrderBook book;
                book.symbol = symbol_;
                book.timestamp = std::stoll(ts_str);

                simdjson::ondemand::document bids_doc = parser.iterate(bids_str);
                for (auto bid_level : bids_doc.get_array())
                {
                    auto level_array = bid_level.get_array();
                    book.bids.push_back({level_array.at(0).get_double(), level_array.at(1).get_double()});
                }

                simdjson::ondemand::document asks_doc = parser.iterate(asks_str);
                for (auto ask_level : asks_doc.get_array())
                {
                    auto level_array = ask_level.get_array();
                    book.asks.push_back({level_array.at(0).get_double(), level_array.at(1).get_double()});
                }

                auto ob_event = std::make_shared<OrderBookEvent>(book);
                event_bus_->publish(ob_event);
            }
            catch (const std::exception &e)
            {
                Log::get_logger()->error("Failed to parse order book data line: {}", e.what());
            }
        }
        Log::get_logger()->info("DataHandler finished processing file: {}.", file_path_);
        event_bus_->publish(std::make_shared<Event>(EventType::SYSTEM));
    }
} // namespace hft_system