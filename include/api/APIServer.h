/**
 * @file APIServer.h
 * @brief Public API declarations for APIServer.
 */

#ifndef HFT_SYSTEM_APISERVER_H
#define HFT_SYSTEM_APISERVER_H

#include <thread>
#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../core/Application.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace hft_system {

class APIServer {
public:
    APIServer(Application& app, const std::string& address, unsigned short port);
    ~APIServer();
/** @brief start. */
    void start();
    void stop();

private:
    void run();
    void do_session(tcp::socket& socket);

    Application& app_;
    net::io_context ioc_{1};
    tcp::acceptor acceptor_;
    std::thread server_thread_;
};

} // namespace hft_system
#endif // HFT_SYSTEM_APISERVER_H