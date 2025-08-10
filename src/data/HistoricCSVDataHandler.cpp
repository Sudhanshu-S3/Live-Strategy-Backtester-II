#include "../../include/data/HistoricCSVDataHandler.h"
#include "../../include/core/Log.h"
#include "simdjson.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>

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

    // Helper function to safely extract JSON strings from CSV
    std::pair<std::string, std::string> extract_json_from_csv_line(const std::string &line)
    {
        // Expected format: timestamp,"[bid_json]","[ask_json]"
        size_t first_quote = line.find('"');
        if (first_quote == std::string::npos)
        {
            throw std::runtime_error("No opening quote found");
        }

        size_t second_quote = line.find('"', first_quote + 1);
        if (second_quote == std::string::npos)
        {
            throw std::runtime_error("No closing quote for bids");
        }

        size_t third_quote = line.find('"', second_quote + 1);
        if (third_quote == std::string::npos)
        {
            throw std::runtime_error("No opening quote for asks");
        }

        size_t fourth_quote = line.find('"', third_quote + 1);
        if (fourth_quote == std::string::npos)
        {
            throw std::runtime_error("No closing quote for asks");
        }

        std::string bids_str = line.substr(first_quote + 1, second_quote - first_quote - 1);
        std::string asks_str = line.substr(third_quote + 1, fourth_quote - third_quote - 1);

        return {bids_str, asks_str};
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

        std::string line;
        std::getline(file, line); // Skip header

        size_t line_number = 1; // Start from 1 since we skipped header

        while (is_running_.load() && std::getline(file, line))
        {
            ++line_number;

            if (line.empty())
            {
                continue; // Skip empty lines
            }

            try
            {
                // Extract timestamp (everything before first comma)
                size_t first_comma = line.find(',');
                if (first_comma == std::string::npos)
                {
                    Log::get_logger()->warn("Line {}: No comma found, skipping", line_number);
                    continue;
                }

                std::string ts_str = line.substr(0, first_comma);

                // Extract JSON strings safely
                auto [bids_str, asks_str] = extract_json_from_csv_line(line);

                // Validate JSON strings are not empty
                if (bids_str.empty() || asks_str.empty())
                {
                    Log::get_logger()->warn("Line {}: Empty JSON data, skipping", line_number);
                    continue;
                }

                OrderBook book;
                book.symbol = symbol_;
                book.timestamp = std::stoll(ts_str);

                // Create separate parsers for each JSON string to avoid reuse issues
                simdjson::ondemand::parser bids_parser;
                simdjson::ondemand::parser asks_parser;

                // Parse bids with error checking
                try
                {
                    simdjson::ondemand::document bids_doc = bids_parser.iterate(bids_str);
                    simdjson::ondemand::array bids_array = bids_doc.get_array();

                    for (auto bid_level_value : bids_array)
                    {
                        simdjson::ondemand::array level_array = bid_level_value.get_array();

                        auto iter = level_array.begin();
                        if (iter == level_array.end())
                            continue;

                        double price = (*iter).get_double();
                        ++iter;
                        if (iter == level_array.end())
                            continue;

                        double quantity = (*iter).get_double();

                        book.bids.push_back({price, quantity});
                    }
                }
                catch (const simdjson::simdjson_error &e)
                {
                    Log::get_logger()->error("Line {}: Failed to parse bids JSON: '{}'. Error: {}",
                                             line_number, bids_str, e.what());
                    continue;
                }

                // Parse asks with error checking
                try
                {
                    simdjson::ondemand::document asks_doc = asks_parser.iterate(asks_str);
                    simdjson::ondemand::array asks_array = asks_doc.get_array();

                    for (auto ask_level_value : asks_array)
                    {
                        simdjson::ondemand::array level_array = ask_level_value.get_array();

                        auto iter = level_array.begin();
                        if (iter == level_array.end())
                            continue;

                        double price = (*iter).get_double();
                        ++iter;
                        if (iter == level_array.end())
                            continue;

                        double quantity = (*iter).get_double();

                        book.asks.push_back({price, quantity});
                    }
                }
                catch (const simdjson::simdjson_error &e)
                {
                    Log::get_logger()->error("Line {}: Failed to parse asks JSON: '{}'. Error: {}",
                                             line_number, asks_str, e.what());
                    continue;
                }

                // Only publish if we have valid data
                if (!book.bids.empty() || !book.asks.empty())
                {
                    auto ob_event = std::make_shared<OrderBookEvent>(book);
                    event_bus_->publish(ob_event);
                }
            }
            catch (const std::exception &e)
            {
                Log::get_logger()->error("Line {}: Failed to parse line: '{}'. Error: {}",
                                         line_number, line, e.what());
                continue;
            }

            // Add a small delay to prevent overwhelming the system
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        Log::get_logger()->info("DataHandler finished processing file: {}. Processed {} lines.", file_path_, line_number);

        // Signal system completion
        auto system_event = std::make_shared<Event>(EventType::SYSTEM);
        event_bus_->publish(system_event);
    }

} // namespace hft_system