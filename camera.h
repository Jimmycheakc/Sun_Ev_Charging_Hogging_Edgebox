#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include "log.h"

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
public:
    // Take ownership of the stream
    session(boost::asio::ip::tcp::socket&& socket)
        : stream_(std::move(socket))
    {

    }

    // Start the asynchronous operation
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        boost::asio::dispatch(stream_.get_executor(),
                        boost::beast::bind_front_handler(
                            &session::do_read,
                            shared_from_this()));
    }

    void do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        boost::beast::http::async_read(stream_, buffer_, req_,
                            boost::beast::bind_front_handler(
                                &session::on_read,
                                shared_from_this()));
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == boost::beast::http::error::end_of_stream)
        {
            return do_close();
        }

        if (ec)
        {
            std::ostringstream oss;
            oss << "Session read Exception :"  << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }

        res_ = {};

        if (req_.method() == boost::beast::http::verb::post)
        {
            // Handle post request
            std::cout << "Request Method :" << req_.method_string() << std::endl;
            std::cout << "Request Target :" << req_.target() << std::endl;
            std::cout << "Request Version :" << req_.version() << std::endl;

            for (const auto& header : req_)
            {
                std::cout << "Header :" << header.name_string() << " = " << header.value() << std::endl;
            }

            if (req_.body().size() > 0)
            {
                std::cout << "Request Body :" << req_.body() << std::endl;
            }

            // Create response
            res_.result(boost::beast::http::status::ok);
            res_.version(req_.version());
            res_.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res_.set(boost::beast::http::field::content_type, "application/json");
            res_.keep_alive(req_.keep_alive());

            // Set the JSON body
            res_.body() = R"({"code": "0", "msg": "success"})";
            res_.content_length(res_.body().size());
            res_.prepare_payload();

            send_response();
        }
        else
        {
            // Create response - unknown http-method
            res_.result(boost::beast::http::status::bad_request);
            res_.version(req_.version());
            res_.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res_.set(boost::beast::http::field::content_type, "application/json");
            res_.keep_alive(req_.keep_alive());

            // Set the JSON body
            res_.body() = R"({"code": "-1", "msg": "fail"})";
            res_.content_length(res_.body().size());
            res_.prepare_payload();

            send_response();
        }
    }

    void send_response()
    {
        bool keep_alive = res_.keep_alive();

        // Write the response
        boost::beast::http::async_write(stream_, res_,
                        boost::beast::bind_front_handler(
                            &session::on_write,
                            shared_from_this(),
                            keep_alive));
    }

    void on_write(bool keep_alive, boost::beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            std::ostringstream oss;
            oss << "Session write Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }

        if (! keep_alive)
        {
            // This means we should close the connection, usually because
            // thre response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }

    void do_close()
    {
        // Send a TCP shutdown
        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }

private:
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
};

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
public:
    listener(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint)
        : io_context_(io_context),
        acceptor_(boost::asio::make_strand(io_context))
    {
        boost::beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec)
        {
            std::ostringstream oss;
            oss << "Listener Open Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            std::ostringstream oss;
            oss << "Listener set_option Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec)
        {
            std::ostringstream oss;
            oss << "Listener bind Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }

        // Start listening for connections
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            std::ostringstream oss;
            oss << "Listener listen Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
            return;
        }
    }

    // Start accepting incoming connections
    void run()
    {
        do_accept();
    }

private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(boost::asio::make_strand(io_context_),
                            boost::beast::bind_front_handler(
                                &listener::on_accept,
                                shared_from_this()));
    }

    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if (ec)
        {
            std::ostringstream oss;
            oss << "Listener accept Exception :" << ec.message();
            Logger::getInstance()->FnLog(oss.str(), "SERVER");
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};

class CameraServer
{

public:
    static CameraServer* getInstance();
    void FnCameraServerInitialization(boost::asio::io_context& io_context, const std::string& address, unsigned short port);

    /*
     * Singleton CameraServer cannot be cloneable
     */
    CameraServer(CameraServer& cameraServer) = delete;

    /*
     * Singleton CameraServer cannot be assignable
     */
    void operator=(const CameraServer&) = delete;

private:
    static CameraServer* cameraServer_;
    static std::mutex mutex_;
    CameraServer();

    std::shared_ptr<listener> pCameraServer_;
};