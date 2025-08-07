#include "../../include/config/ConfigParser.h"
#include "simdjson.h"
#include <stdexcept>
#include <string_view>

namespace hft_system
{

    Config ConfigParser::parse(const std::string &filename)
    {
        Config config;
        simdjson::ondemand::parser parser;
        simdjson::padded_string json;

        auto error = simdjson::padded_string::load(filename).get(json);
        if (error)
        {
            throw std::runtime_error("Could not load config file: " + filename);
        }

        simdjson::ondemand::document doc = parser.iterate(json);

        // Safely parse top-level fields
        std::string_view mode_str;
        if (doc["run_mode"].get_string().get(mode_str) == simdjson::SUCCESS)
        {
            if (mode_str == "LIVE")
                config.run_mode = RunMode::LIVE;
            else
                config.run_mode = RunMode::BACKTEST;
        }

        doc["initial_capital"].get_double().get(config.initial_capital);

        // Safely parse nested objects
        simdjson::ondemand::object data_obj;
        if (doc["data"].get_object().get(data_obj) == simdjson::SUCCESS)
        {
            std::string_view symbol, file_path;
            data_obj["symbol"].get_string().get(symbol);
            data_obj["data_file"].get_string().get(file_path);
            config.data.symbol = symbol;
            config.data.file_path = file_path;
        }

        simdjson::ondemand::object ws_obj;
        if (doc["websocket"].get_object().get(ws_obj) == simdjson::SUCCESS)
        {
            std::string_view host, target;
            ws_obj["host"].get_string().get(host);

            // **THE FIX IS HERE:** Use a temporary int64_t variable for parsing.
            int64_t port_val;
            ws_obj["port"].get_int64().get(port_val);
            config.websocket.port = static_cast<int>(port_val);

            ws_obj["target"].get_string().get(target);
            config.websocket.host = host;
            config.websocket.target = target;
        }

        simdjson::ondemand::array strategies_array;
        if (doc["strategies"].get_array().get(strategies_array) == simdjson::SUCCESS)
        {
            for (auto strategy_obj_val : strategies_array)
            {
                simdjson::ondemand::object strategy_obj = strategy_obj_val.get_object();
                StrategyConfig sc;
                std::string_view name, symbol;
                strategy_obj["name"].get_string().get(name);
                strategy_obj["symbol"].get_string().get(symbol);
                sc.name = name;
                sc.symbol = symbol;

                simdjson::ondemand::object params_obj;
                if (strategy_obj["params"].get_object().get(params_obj) == simdjson::SUCCESS)
                {
                    // **THE FIX IS HERE:** Use a temporary int64_t variable for parsing.
                    int64_t lookback_val;
                    params_obj["lookback_levels"].get_int64().get(lookback_val);
                    sc.params.lookback_levels = static_cast<int>(lookback_val);

                    params_obj["imbalance_threshold"].get_double().get(sc.params.imbalance_threshold);
                }
                config.strategies.push_back(sc);
            }
        }

        return config;
    }

} // namespace hft_system