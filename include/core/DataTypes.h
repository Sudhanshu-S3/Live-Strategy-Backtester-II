#ifndef HFT_SYSTEM_DATATYPES_H
#define HFT_SYSTEM_DATATYPES_H

#include <string>
#include <vector>

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

    // Represents a single level (price and quantity) in the order book.
    struct OrderBookLevel
    {
        double price;
        double quantity;
    };

    // Represents a full snapshot of the order book at a point in time.
    struct OrderBook
    {
        std::string symbol;
        long long timestamp = 0;
        std::vector<OrderBookLevel> bids;
        std::vector<OrderBookLevel> asks;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_DATATYPES_H