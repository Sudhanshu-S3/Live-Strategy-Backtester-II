#include "../../include/config/ConfigParser.h"
#include "simdjson.h"
#include <stdexcept>

namespace hft_system
{

    Config ConfigParser::parse(const std::string &filename)
    {
        Config config;
        simdjson::ondemand::parser parser;
        simdjson::padded_string json;

        // Load the file into a padded string
        auto error = simdjson::padded_string::load(filename).get(json);
        if (error)
        {
            throw std::runtime_error("Could not load config file: " + filename);
        }

        // Iterate over the document
        simdjson::ondemand::document doc = parser.iterate(json);

        // Extract values, throwing an error if a key is missing or has the wrong type.
        config.initial_capital = doc["initial_capital"].get_double();

        simdjson::ondemand::object data_obj = doc["data"].get_object();
        config.data.symbol = data_obj["symbol"].get_string().value();
        config.data.file_path = data_obj["data_file"].get_string().value();

        if (doc.find_field("execution").error() == simdjson::SUCCESS)
        {
            simdjson::ondemand::object exec_obj = doc["execution"].get_object();
            config.execution.commission_pct = exec_obj["commission_pct"].get_double();
            config.execution.slippage_pct = exec_obj["slippage_pct"].get_double();
        }
        if (doc.find_field("risk").error() == simdjson::SUCCESS)
        {
            simdjson::ondemand::object risk_obj = doc["risk"].get_object();
            config.risk.risk_per_trade_pct = risk_obj["risk_per_trade_pct"].get_double();
        }

        return config;
    }

} // namespace hft_system