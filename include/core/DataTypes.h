// include/core/DataTypes.h
#ifndef HFT_SYSTEM_DATATYPES_H
#define HFT_SYSTEM_DATATYPES_H

namespace hft_system
{

    enum class OrderDirection
    {
        BUY,
        SELL,
        NONE
    };

    enum class OrderType
    {
        MARKET,
        LIMIT
    };

} // namespace hft_system

#endif // HFT_SYSTEM_DATATYPES_H