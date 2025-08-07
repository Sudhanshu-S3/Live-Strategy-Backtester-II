#include "../../include/data/WebSocketDataHandler.h"
#include "../../include/core/Log.h"
#include "simdjson.h"

namespace hft_system
{

    // Helper function to report errors
    void fail(beast::error_code ec, char const *what)
    {
        Log::get_logger()->error("{}: {}", what, ec.message());
    }

    WebSocketDataHandler::WebSocketDataHandler(std::shared_ptr<EventBus> event_bus, const WebSocketConfig &config)
        : DataHandler(event_bus, "WebSocketDataHandler"),
          config_(config),
          resolver_(net::make_strand(ioc_)),
          ws_(net::make_strand(ioc_), ctx_) {}

    WebSocketDataHandler::~WebSocketDataHandler()
    {
        stop();
    }

    void WebSocketDataHandler::start()
    {
        Log::get_logger()->info("Connecting to {}:{}", config_.host, config_.port);
        resolver_.async_resolve(config_.host, std::to_string(config_.port),
                                beast::bind_front_handler(&WebSocketDataHandler::on_resolve, shared_from_this()));
        ioc_thread_ = std::thread([this]()
                                  { ioc_.run(); });
    }

    void WebSocketDataHandler::stop()
    {
        if (!ioc_.stopped())
        {
            ioc_.post([this]()
                      {
            beast::error_code ec;
            ws_.close(websocket::close_code::normal, ec);
            if (ec) fail(ec, "close"); });
        }
        if (ioc_thread_.joinable())
        {
            ioc_thread_.join();
        }
    }

    void WebSocketDataHandler::run() {} // Async operations are managed by io_context

    void WebSocketDataHandler::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec)
            return fail(ec, "resolve");

        net::async_connect(
            beast::get_lowest_layer(ws_),
            results.begin(), results.end(),
            beast::bind_front_handler(&WebSocketDataHandler::on_connect, shared_from_this()));
    }

    void WebSocketDataHandler::on_connect(beast::error_code ec, tcp::resolver::results_type::iterator)
    {
        if (ec)
            return fail(ec, "connect");

        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), config_.host.c_str()))
        {
            ec = beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category());
            return fail(ec, "set_sni");
        }

        ws_.next_layer().async_handshake(ssl::stream_base::client,
                                         beast::bind_front_handler(&WebSocketDataHandler::on_ssl_handshake, shared_from_this()));
    }

    void WebSocketDataHandler::on_ssl_handshake(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "ssl_handshake");
        ws_.async_handshake(config_.host, config_.target,
                            beast::bind_front_handler(&WebSocketDataHandler::on_handshake, shared_from_this()));
    }

    void WebSocketDataHandler::on_handshake(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "handshake");
        Log::get_logger()->info("WebSocket Handshake successful.");

        ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketDataHandler::on_read, shared_from_this()));
    }

    void WebSocketDataHandler::on_write(beast::error_code ec, std::size_t)
    {
        if (ec)
            return fail(ec, "write");
        buffer_.clear();
        ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketDataHandler::on_read, shared_from_this()));
    }

    void WebSocketDataHandler::on_read(beast::error_code ec, std::size_t)
    {
        if (ec)
            return fail(ec, "read");

        std::string message = beast::buffers_to_string(buffer_.data());
        Log::get_logger()->trace("Received WS message: {}", message);
        process_message(message);

        buffer_.consume(buffer_.size());
        ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketDataHandler::on_read, shared_from_this()));
    }

    void WebSocketDataHandler::on_close(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "close");
        Log::get_logger()->info("WebSocket connection closed.");
    }

    void WebSocketDataHandler::process_message(const std::string &message)
    {
        // Placeholder for simdjson parsing and publishing OrderBookEvents
    }

} // namespace hft_system