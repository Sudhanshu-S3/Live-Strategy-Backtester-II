#ifndef HFT_SYSTEM_UTILS_H
#define HFT_SYSTEM_UTILS_H

#include "Log.h"
#include <boost/beast/core.hpp>

namespace hft_system {

// Use 'inline' to allow this function to be defined in a header
// without causing multiple definition errors.
inline void fail(boost::beast::error_code ec, char const* what) {
    if (ec) {
        Log::get_logger()->error("{}: {}", what, ec.message());
    }
}

} // namespace hft_system

#endif // HFT_SYSTEM_UTILS_H