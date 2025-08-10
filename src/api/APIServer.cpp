#include "../../include/api/APIServer.h"
#include "../../include/core/Log.h"
#include "../../include/core/Utils.h"
#include <sstream> // Include for std::stringstream
#include <iomanip> // Include for std::fixed and std::setprecision

namespace hft_system
{

    APIServer::APIServer(Application &app, const std::string &address, unsigned short port)
        : app_(app), acceptor_(ioc_, {net::ip::make_address(address), port}) {}

    APIServer::~APIServer()
    {
        stop();
    }

    void APIServer::start()
    {
        server_thread_ = std::thread(&APIServer::run, this);
    }

    void APIServer::stop()
    {
        ioc_.stop();
        if (server_thread_.joinable())
        {
            server_thread_.join();
        }
    }

    void APIServer::run()
    {
        try
        {
            Log::get_logger()->info("API Server starting on port {}", acceptor_.local_endpoint().port());
            while (!ioc_.stopped())
            {
                tcp::socket socket{ioc_};
                acceptor_.accept(socket);
                do_session(socket);
            }
        }
        catch (const std::exception &e)
        {
            Log::get_logger()->error("API Server exception: {}", e.what());
        }
    }

    void APIServer::do_session(tcp::socket &socket)
    {
        beast::error_code ec;
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);

        if (ec == http::error::end_of_stream)
            return;
        if (ec)
            return fail(ec, "read");

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "HFT_System");
        res.set(http::field::content_type, "application/json");
        res.set(http::field::access_control_allow_origin, "*");
        res.keep_alive(req.keep_alive());

        if (req.method() == http::verb::post && req.target() == "/start")
        {
            Log::get_logger()->info("API: Received /start request.");
            app_.run();
            res.body() = R"({"message":"Backtester started"})";
        }
        else if (req.method() == http::verb::post && req.target() == "/stop")
        {
            Log::get_logger()->info("API: Received /stop request.");
            app_.stop();
            res.body() = R"({"message":"Backtester stopped"})";
        }
        else if (req.method() == http::verb::get && req.target() == "/report")
        {
            Log::get_logger()->info("API: Received /report request.");
            auto report_data = app_.get_analytics_report();

            // Manually build the JSON response string.
            std::stringstream ss;
            ss << std::fixed << std::setprecision(4) << "{";
            for (auto it = report_data.begin(); it != report_data.end(); ++it)
            {
                ss << "\"" << it->first << "\":" << it->second;
                if (std::next(it) != report_data.end())
                {
                    ss << ",";
                }
            }
            ss << "}";
            res.body() = ss.str();
        }
        else if (req.method() == http::verb::get && req.target() == "/pnl")
        {
            Log::get_logger()->info("API: Received /pnl request.");
            auto pnl_data = app_.get_pnl_snapshot();
            std::stringstream ss;
            ss << std::fixed << std::setprecision(4) << "{";
            for (auto it = pnl_data.begin(); it != pnl_data.end(); ++it)
            {
                ss << "\"" << it->first << "\":" << it->second;
                if (std::next(it) != pnl_data.end())
                {
                    ss << ",";
                }
            }
            ss << "}";
            res.body() = ss.str();
        }
        else
        {
            res.result(http::status::not_found);
            res.body() = R"({"error":"Not Found"})";
        }

        res.prepare_payload();
        http::write(socket, res, ec);
        if (ec)
            return fail(ec, "write");
    }

} // namespace hft_system