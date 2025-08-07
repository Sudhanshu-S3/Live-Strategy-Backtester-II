#ifndef HFT_SYSTEM_WEBSOCKETDATAHANDLER_H
#define HFT_SYSTEM_WEBSOCKETDATAHANDLER_H

#include "DataHandler.h"
#include "../config/Config.h"
#include <thread>
#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

// **THE FIX IS HERE:** Add the namespace wrapper
namespace hft_system
{

    class WebSocketDataHandler : public DataHandler, public std::enable_shared_from_this<WebSocketDataHandler>
    {
    public:
        WebSocketDataHandler(std::shared_ptr<EventBus> event_bus, const WebSocketConfig &config);
        ~WebSocketDataHandler();

        void start() override;
        void stop() override;
        void run() override;

    private:
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::iterator endpoint_iter);
        void on_ssl_handshake(beast::error_code ec);
        void on_handshake(beast::error_code ec);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_close(beast::error_code ec);
        void process_message(const std::string &message);

        WebSocketConfig config_;
        net::io_context ioc_;
        ssl::context ctx_{ssl::context::tlsv12_client};
        tcp::resolver resolver_;
        websocket::stream<beast::ssl_stream<tcp::socket>> ws_;
        beast::flat_buffer buffer_;
        std::thread ioc_thread_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_WEBSOCKETDATAHANDLER_H