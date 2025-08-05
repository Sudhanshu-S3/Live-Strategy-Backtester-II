#include "../../include/config/ConfigParser.h"
#include "simdjson.h"
#include <stdexcept>

namespace hft_system
{

    Config ConfigParser::parse(const std::string &filename)
    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string json = simdjson::padded_string::load(filename);
        simdjson::ondemand::document doc = parser.iterate(json);

        Config config;

        // Extract values, throwing an error if a key is missing or has the wrong type.
        config.initial_capital = doc["initial_capital"].get_double();

        simdjson::ondemand::object data_obj = doc["data"].get_object();
        config.data.symbol = data_obj["symbol"].get_string().value();
        config.data.file_path = data_obj["data_file"].get_string().value();

        return config;
    }

} // namespace hft_system