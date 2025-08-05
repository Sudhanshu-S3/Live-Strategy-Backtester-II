#ifndef HFT_SYSTEM_CONFIGPARSER_H
#define HFT_SYSTEM_CONFIGPARSER_H

#include "Config.h"
#include <string>

namespace hft_system
{

    class ConfigParser
    {
    public:
        // Parses the given JSON file and returns a Config object.
        // Throws an exception on error.
        static Config parse(const std::string &filename);
    };

} // namespace hft_system

#endif // HFT_SYSTEM_CONFIGPARSER_H