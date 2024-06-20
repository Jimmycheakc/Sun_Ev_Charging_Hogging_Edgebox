#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include "log.h"

class httpClientSession : public std::enable_shared_from_this<httpClientSession>
{
public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit httpClientSession(boost::asio::io_context& ioc, boost::asio::strand<boost::asio::io_context::executor_type>& strand, std::function<void(boost::beast::error_code ec, const std::string& msg)> callback)
        : resolver_(strand),
        stream_(strand),
        callback(callback)
    {
    }

    // Start the asynchronous operation
    void run(const std::string& host, const std::string& port,
            const std::string& target, int version,
            const std::string& jsonBody)
    {
        req_.version(version);
        req_.method(boost::beast::http::verb::post);
        req_.target(target);
        req_.set(boost::beast::http::field::host, host);
        req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(boost::beast::http::field::content_type, "application/json");
        req_.body() = jsonBody;
        req_.prepare_payload();

        // Look up the domain name
        resolver_.async_resolve(host, port, 
                            boost::beast::bind_front_handler(
                            &httpClientSession::on_resolve, shared_from_this()));
    }

    void on_resolve(boost::beast::error_code ec, 
                boost::asio::ip::tcp::resolver::results_type results)
    {
        if (ec)
        {
            return callback(ec, "Resolve Error");
        }

        // Set a timeout on the operation
        stream_.expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        stream_.async_connect(results,
                        boost::beast::bind_front_handler(
                        &httpClientSession::on_connect, shared_from_this()));
    }

    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type)
    {
        if (ec)
        {
            return callback(ec, "Connect Error");
        }

        // Output the JSON body
        //Logger::getInstance()->FnLog("Request JSON Body :" + req_.body(), "CENTRAL");

        // Set a timeout on the operation
        stream_.expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(stream_, req_,
                                boost::beast::bind_front_handler(
                                &httpClientSession::on_write, shared_from_this()));
    }

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            return callback(ec, "Write Error");
        }

        boost::beast::http::async_read(stream_, buffer_, res_,
                                boost::beast::bind_front_handler(
                                &httpClientSession::on_read, shared_from_this()));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            return callback(ec, "Read Error");
        }

        // Log the response
        std::string responseBody = res_.body();
        Logger::getInstance()->FnLog(responseBody, "CENTRAL");

        // Gracefully close the socket
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // not_connected happens sometimess so don't bother reporting it.
        if (ec && ec != boost::beast::errc::not_connected)
        {
            return callback(ec, "Shutdown Error");
        }

        if (res_.result() == boost::beast::http::status::ok)
        {
            return callback(ec, "");
        }
        else
        {
            return callback(ec, "Status Not Ok");
        }
    }

private:
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    std::function<void(boost::beast::error_code ec, const std::string& msg)> callback;
};


class Central
{

public:
    static const std::string ERROR_CODE_RECOVERED;
    static const std::string ERROR_CODE_CAMERA;
    static const std::string ERROR_CODE_IPC;

    const std::string USERNAME = "testuser";
    const std::string PASSWORD = "testpassword";

    static Central* getInstance();

    void FnCentralInitialization(boost::asio::io_context& io_context, boost::asio::strand<boost::asio::io_context::executor_type>& strand);
    void FnSendHeartbeatUpdate();
    void FnSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code);
    void FnSendParkInParkOutInfo(const std::string& lot_no,
                                const std::string& lpn,
                                const std::string& lot_in_image,
                                const std::string& lot_out_image,
                                const std::string& lot_in_time,
                                const std::string& lot_out_time);

    void FnSetCentralStatus(bool status);
    bool FnGetCentralStatus();

    /*
     * Singleton Central cannot be cloneable
     */
    Central(Central &central) = delete;

    /*
     * Singleton Central cannot be assignable
     */
    void operator=(const Central &) = delete;

private:
    static Central* central_;
    static std::mutex mutex_;
    Central();
    std::atomic<bool> centralStatus_;
    std::string centralServerIp_;
    int centralServerPort_;
    std::shared_ptr<httpClientSession> pSendDeviceStatusSession_;
    std::shared_ptr<httpClientSession> pSendHeartbeatSession_;
    std::shared_ptr<httpClientSession> pSendParkInParkOutSession_;

    void onSendHeartbeatUpdateCallbackHandler(boost::beast::error_code ec, const std::string& msg);
    void onSendDeviceStatusUpdateCallbackHandler(boost::beast::error_code ec, const std::string& msg);
    void onSendParkInParkOutCallbackHandler(boost::beast::error_code ec, const std::string& msg);
};