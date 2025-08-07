#ifndef HFT_SYSTEM_CONFIGPARSER_H
#define HFT_SYSTEM_CONFIGPARSER_H

#include "Config.h"
#include <string>

namespace hft_system
{

    class ConfigParser
    {
    public:
        static Config parse(const std::string &filename);
    };

} // namespace hft_system

#endif // HFT_SYSTEM_CONFIGPARSER_H