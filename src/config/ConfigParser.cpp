#include "../../include/config/ConfigParser.h"
#include "simdjson.h"
#include <stdexcept>
#include <string_view>
#include <map>

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
            else if (mode_str == "OPTIMIZATION")
                config.run_mode = RunMode::OPTIMIZATION;
            else if (mode_str == "WALK_FORWARD")
                config.run_mode = RunMode::WALK_FORWARD; // Add this
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

        // ... (Add safe parsing for other sections like execution, risk, analytics as needed)

        simdjson::ondemand::object ws_obj;
        if (doc["websocket"].get_object().get(ws_obj) == simdjson::SUCCESS)
        {
            std::string_view host, target, symbol;
            ws_obj["host"].get_string().get(host);
            ws_obj["target"].get_string().get(target);
            ws_obj["symbol"].get_string().get(symbol);

            int64_t port_val;
            ws_obj["port"].get_int64().get(port_val);
            config.websocket.port = static_cast<int>(port_val);

            config.websocket.host = host;
            config.websocket.target = target;
            config.websocket.symbol = symbol;
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
                    int64_t lookback_val;
                    params_obj["lookback_levels"].get_int64().get(lookback_val);
                    sc.params.lookback_levels = static_cast<int>(lookback_val);
                    params_obj["imbalance_threshold"].get_double().get(sc.params.imbalance_threshold);
                }
                config.strategies.push_back(sc);
            }
        }

        simdjson::ondemand::object opt_obj;
        if (doc["optimization"].get_object().get(opt_obj) == simdjson::SUCCESS)
        {
            std::string_view strategy_name;
            opt_obj["strategy_name"].get_string().get(strategy_name);
            config.optimization.strategy_name = strategy_name;

            simdjson::ondemand::object ranges_obj;
            if (opt_obj["param_ranges"].get_object().get(ranges_obj) == simdjson::SUCCESS)
            {
                for (auto field : ranges_obj)
                {
                    std::string_view key = field.key();
                    std::vector<double> values;
                    for (auto val : field.value().get_array())
                    {
                        values.push_back(val.get_double());
                    }
                    config.optimization.param_ranges[std::string(key)] = values;
                }
            }
        }

        simdjson::ondemand::object wf_obj;
        if (doc["walk_forward"].get_object().get(wf_obj) == simdjson::SUCCESS)
        {
            std::string_view start_date, end_date;
            wf_obj["start_date"].get_string().get(start_date);
            wf_obj["end_date"].get_string().get(end_date);

            int64_t in_sample_days, out_of_sample_days;
            wf_obj["in_sample_days"].get_int64().get(in_sample_days);
            wf_obj["out_of_sample_days"].get_int64().get(out_of_sample_days);

            config.walk_forward.start_date = start_date;
            config.walk_forward.end_date = end_date;
            config.walk_forward.in_sample_days = in_sample_days;
            config.walk_forward.out_of_sample_days = out_of_sample_days;
        }

        return config;
    }

} // namespace hft_system