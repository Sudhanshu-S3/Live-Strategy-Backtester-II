#include "../../include/data/WebSocketDataHandler.h"
#include "../../include/core/Log.h"
#include "simdjson.h"
#include <algorithm>
#include "../../include/core/Utils.h"
#include <string>

namespace hft_system
{

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
            net::dispatch(ioc_, [this]()
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

        // **THE FIX IS HERE:** Manually build the JSON subscription string.
        std::string symbol_lower = config_.symbol;
        std::transform(symbol_lower.begin(), symbol_lower.end(), symbol_lower.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        std::string subscribe_msg =
            R"({"method":"SUBSCRIBE","params":[")" +
            symbol_lower +
            R"(@depth"],"id":1})";

        Log::get_logger()->info("Sending subscription message: {}", subscribe_msg);

        ws_.async_write(net::buffer(subscribe_msg),
                        beast::bind_front_handler(&WebSocketDataHandler::on_write, shared_from_this()));
    }

    void WebSocketDataHandler::on_write(beast::error_code ec, std::size_t)
    {
        if (ec)
            return fail(ec, "write");
        ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketDataHandler::on_read, shared_from_this()));
    }

    void WebSocketDataHandler::on_read(beast::error_code ec, std::size_t)
    {
        if (ec)
            return fail(ec, "read");

        std::string message = beast::buffers_to_string(buffer_.data());
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
        try
        {
            simdjson::padded_string padded_message(message);
            simdjson::ondemand::parser parser;
            simdjson::ondemand::document doc = parser.iterate(padded_message);

            // Check for subscription confirmation first
            if (doc.find_field("result").error() == simdjson::SUCCESS)
            {
                Log::get_logger()->info("Subscription confirmed.");
                return;
            }

            // --- THE FIX IS HERE ---
            // We now parse the data from the top level of the document, not a nested "data" object.
            std::string_view event_type;
            if (doc["e"].get_string().get(event_type) != simdjson::SUCCESS || event_type != "depthUpdate")
            {
                return; // Not an order book message, ignore it.
            }

            OrderBook book;
            std::string_view symbol_sv;
            doc["s"].get_string().get(symbol_sv);
            book.symbol = symbol_sv;
            book.timestamp = doc["E"].get_int64();

            for (auto bid_level : doc["b"].get_array())
            {
                auto level_array = bid_level.get_array();
                double price = std::stod(std::string(level_array.at(0).get_string().value()));
                double qty = std::stod(std::string(level_array.at(1).get_string().value()));
                if (qty > 1e-9)
                    book.bids.push_back({price, qty});
            }

            for (auto ask_level : doc["a"].get_array())
            {
                auto level_array = ask_level.get_array();
                double price = std::stod(std::string(level_array.at(0).get_string().value()));
                double qty = std::stod(std::string(level_array.at(1).get_string().value()));
                if (qty > 1e-9)
                    book.asks.push_back({price, qty});
            }
            // --- END OF FIX ---

            if (!book.bids.empty() || !book.asks.empty())
            {
                auto ob_event = std::make_shared<OrderBookEvent>(book);
                event_bus_->publish(ob_event);
            }
        }
        catch (const std::exception &e)
        {
            Log::get_logger()->error("Error parsing WebSocket message: {}", e.what());
        }
    }

} // namespace hft_system