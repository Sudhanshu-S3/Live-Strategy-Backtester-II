/**
 * @file ConfigParser.h
 * @brief Public API declarations for ConfigParser.
 */

#ifndef HFT_SYSTEM_CONFIGPARSER_H
#define HFT_SYSTEM_CONFIGPARSER_H

#include "Config.h"
#include <string>

namespace hft_system
{

    class ConfigParser
    {
    public:
/** @brief parse. */
        static Config parse(const std::string &filename);
    };

} // namespace hft_system

#endif // HFT_SYSTEM_CONFIGPARSER_H