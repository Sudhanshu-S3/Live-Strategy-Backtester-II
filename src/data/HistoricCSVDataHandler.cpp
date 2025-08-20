#include "../../include/data/HistoricCSVDataHandler.h"
#include "../../include/core/Log.h"
#include "../../include/utils/Timer.h"
#include "../../include/utils/PerformanceMonitor.h"
#include "simdjson.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

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

    // Add a CSV parsing function that handles quotes properly
    std::vector<std::string> parse_csv_line(const std::string &line)
    {
        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;

        for (char c : line)
        {
            if (c == '"')
            {
                in_quotes = !in_quotes;
            }
            else if (c == ',' && !in_quotes)
            {
                fields.push_back(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }

        // Add the last field
        fields.push_back(field);

        // Remove quotes from fields
        for (auto &f : fields)
        {
            if (f.size() >= 2 && f.front() == '"' && f.back() == '"')
            {
                f = f.substr(1, f.size() - 2);
            }
        }

        return fields;
    }

    void HistoricCSVDataHandler::run()
    {
        Log::get_logger()->info("DataHandler thread started for symbol {} from file {}.", symbol_, file_path_);

        std::ifstream file(file_path_);
        if (!file.is_open())
        {
            Log::get_logger()->error("Failed to open file: {}", file_path_);
            return;
        }

        std::string line;
        int line_number = 0;
        bool header_skipped = false;

        while (is_running_.load() && std::getline(file, line))
        {
            ++line_number;

            if (line.empty())
            {
                continue; // Skip empty lines
            }

            if (!header_skipped)
            {
                header_skipped = true;
                continue; // Skip header row
            }

            try
            {
                TIME_FUNCTION("HistoricCSVDataHandler_process_line");

                auto fields = parse_csv_line(line);

                if (fields.size() < 6)
                {
                    Log::get_logger()->error("Line {}: Not enough fields: {}", line_number, line);
                    continue;
                }

                // Process the fields and create a market event
                long timestamp = std::stol(fields[0]);
                double open = std::stod(fields[1]);
                double high = std::stod(fields[2]);
                double low = std::stod(fields[3]);
                double close = std::stod(fields[4]);
                double volume = std::stod(fields[5]);

                auto market_event = std::make_shared<MarketEvent>(symbol_, close);

                event_bus_->publish(market_event);
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

        Log::get_logger()->info("DataHandler finished processing file: {}. Processed {} lines.",
                                file_path_, line_number);

        // Signal system completion
        auto system_event = std::make_shared<Event>(EventType::SYSTEM);
        event_bus_->publish(system_event);
    }

} // namespace hft_system