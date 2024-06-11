#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/json/src.hpp>
#include <string>
#include <sstream>
#include <vector>
#include "central.h"
#include "common.h"
#include "ini_parser.h"
#include "log.h"


Central* Central::central_ = nullptr;
std::mutex Central::mutex_;

const std::string Central::ERROR_CODE_RECOVERED = "0";
const std::string Central::ERROR_CODE_CAMERA = "1";
const std::string Central::ERROR_CODE_IPC = "2";

Central::Central()
    : io_context_(),
    resolver_(io_context_),
    stream_(io_context_),
    centralStatus_(false)
{
    centralServerIp_ = IniParser::getInstance()->FnGetCentralIP();
    centralServerPort_ = IniParser::getInstance()->FnGetCentralServerPort();
}

Central* Central::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (central_ == nullptr)
    {
        central_ = new Central();
    }
    return central_;
}

bool Central::doSendHeartbeatUpdate()
{
    try
    {
        auto const results = resolver_.resolve(centralServerIp_, std::to_string(centralServerPort_));

        // Set a timeout for the connection operation
        stream_.expires_after(std::chrono::seconds(10));

        // Make the connection on the IP address we get from a lookup
        stream_.connect(results);

        // Set up and HTTP POST request message
        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, "/HeartBeat", 11};
        req.set(boost::beast::http::field::host, centralServerIp_);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::content_type, "application/json");

        boost::json::object jsonObject;
        jsonObject["username"] = USERNAME;
        jsonObject["password"] = PASSWORD;
        jsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
        jsonObject["heartbeat_dt"] = Common::getInstance()->FnGetDateTimeFormat_YYYY_MM_DD_HH_MM_SS();
        jsonObject["msg"] = "Heartbeat Update";

        std::string body = boost::json::serialize(jsonObject);
        req.body() = body;
        req.prepare_payload();

        // Log the Json request
        std::ostringstream oss;
        oss << "Json Request: " << body;
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");

        // Set a timeout for the write operation
        stream_.expires_after(std::chrono::seconds(10));

        // Send the HTTP request to the remote host
        boost::beast::http::write(stream_, req);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        // Set a timeout for the read operation
        stream_.expires_after(std::chrono::seconds(10));

        // Receive the HTTP response
        boost::beast::http::read(stream_, buffer, res);

        // Log the response
        std::string responseBody = boost::beast::buffers_to_string(res.body().data());
        Logger::getInstance()->FnLog(responseBody, "CENTRAL");

        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // If we get here the the connection is closed gracefully
        if (res.result() == boost::beast::http::status::ok)
        {
            return true;
        }
    }
    catch (const boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::timed_out)
        {
            Logger::getInstance()->FnLog("Operation timed out", "CENTRAL");
        }
        else
        {
            std::stringstream ss;
            ss << "Boost.Asio Exception: " << e.what();
            Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
        }
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Exception: " << e.what();
        Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
    }

    return false;
}

bool Central::FnSendHeartbeatUpdate()
{
    int retry = 3;

    while (retry > 0)
    {
        if (doSendHeartbeatUpdate())
        {
            return true;
        }
        else
        {
            retry--;
        }
    }

    return false;
}

bool Central::doSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code)
{
    try
    {
        auto const results = resolver_.resolve(centralServerIp_, std::to_string(centralServerPort_));

        // Set a timeout for the connection operation
        stream_.expires_after(std::chrono::seconds(10));

        // Make the connection on the IP address we get from a lookup
        stream_.connect(results);

        // Set up and HTTP POST request message
        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, "/DeviceStatus", 11};
        req.set(boost::beast::http::field::host, centralServerIp_);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::content_type, "application/json");

        boost::json::object jsonObject;
        jsonObject["username"] = USERNAME;
        jsonObject["password"] = PASSWORD;
        jsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
        jsonObject["device_ip"] = device_ip;
        jsonObject["error_code"] = error_code;

        std::string body = boost::json::serialize(jsonObject);
        req.body() = body;
        req.prepare_payload();

        // Log the Json request
        std::ostringstream oss;
        oss << "Json Request: " << body;
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");

        // Set a timeout for the write operation
        stream_.expires_after(std::chrono::seconds(10));

        // Send the HTTP request to the remote host
        boost::beast::http::write(stream_, req);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        // Set a timeout for the read operation
        stream_.expires_after(std::chrono::seconds(10));

        // Receive the HTTP response
        boost::beast::http::read(stream_, buffer, res);

        // Log the response
        std::string responseBody = boost::beast::buffers_to_string(res.body().data());
        Logger::getInstance()->FnLog(responseBody, "CENTRAL");

        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // If we get here the the connection is closed gracefully
        if (res.result() == boost::beast::http::status::ok)
        {
            return true;
        }
    }
    catch (const boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::timed_out)
        {
            Logger::getInstance()->FnLog("Operation timed out", "CENTRAL");
        }
        else
        {
            std::stringstream ss;
            ss << "Boost.Asio Exception: " << e.what();
            Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
        }
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Exception: " << e.what();
        Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
    }

    return false;
}

bool Central::FnSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code)
{
    int retry = 3;

    while (retry > 0)
    {
        if (doSendDeviceStatusUpdate(device_ip, error_code))
        {
            return true;
        }
        else
        {
            retry--;
        }
    }

    return false;
}

bool Central::doSendParkInParkOutInfo(const std::string& lot_no,
                                const std::string& lpn,
                                const std::string& lot_in_image,
                                const std::string& lot_out_image,
                                const std::string& lot_in_time,
                                const std::string& lot_out_time)
{
    try
    {
        auto const results = resolver_.resolve(centralServerIp_, std::to_string(centralServerPort_));

        // Set a timeout for the connection operation
        stream_.expires_after(std::chrono::seconds(10));

        // Make the connection on the IP address we get from a lookup
        stream_.connect(results);

        // Set up and HTTP POST request message
        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post, "/ParkInOut", 11};
        req.set(boost::beast::http::field::host, centralServerIp_);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::content_type, "application/json");

        boost::json::object jsonObject;
        jsonObject["username"] = USERNAME;
        jsonObject["password"] = PASSWORD;
        jsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
        jsonObject["lot_no"] = lot_no;
        jsonObject["lpn"] = lpn;
        jsonObject["lot_in_image"] = lot_in_image;
        jsonObject["lot_out_image"] = lot_out_image;
        jsonObject["lot_in_time"] = lot_in_time;
        jsonObject["lot_out_time"] = lot_out_time;

        std::string body = boost::json::serialize(jsonObject);
        req.body() = body;
        req.prepare_payload();

        // Log the Json request
        boost::json::object outputJsonObject;
        outputJsonObject["username"] = USERNAME;
        outputJsonObject["password"] = PASSWORD;
        outputJsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
        outputJsonObject["lot_no"] = lot_no;
        outputJsonObject["lpn"] = lpn;
        outputJsonObject["lot_in_image"] = (lot_in_image.empty()) ? "Empty" : "Lot In Image";
        outputJsonObject["lot_out_image"] = (lot_out_image.empty()) ? "Empty" : "Lot out Image";
        outputJsonObject["lot_in_time"] = lot_in_time;
        outputJsonObject["lot_out_time"] = lot_out_time;

        std::string outputBody = boost::json::serialize(outputJsonObject);

        std::ostringstream oss;
        oss << "Json Request: " << outputJsonObject;
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");

        // Set a timeout for the write operation
        stream_.expires_after(std::chrono::seconds(10));

        // Send the HTTP request to the remote host
        boost::beast::http::write(stream_, req);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        // Set a timeout for the read operation
        stream_.expires_after(std::chrono::seconds(10));

        // Receive the HTTP response
        boost::beast::http::read(stream_, buffer, res);

        // Log the response
        std::string responseBody = boost::beast::buffers_to_string(res.body().data());
        Logger::getInstance()->FnLog(responseBody, "CENTRAL");

        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // If we get here the the connection is closed gracefully
        if (res.result() == boost::beast::http::status::ok)
        {
            return true;
        }
    }
    catch (const boost::system::system_error& e)
    {
        if (e.code() == boost::asio::error::timed_out)
        {
            Logger::getInstance()->FnLog("Operation timed out", "CENTRAL");
        }
        else
        {
            std::stringstream ss;
            ss << "Boost.Asio Exception: " << e.what();
            Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
        }
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Exception: " << e.what();
        Logger::getInstance()->FnLog(ss.str(), "CENTRAL");
    }

    return false;
}

bool Central::FnSendParkInParkOutInfo(const std::string& lot_no,
                                const std::string& lpn,
                                const std::string& lot_in_image,
                                const std::string& lot_out_image,
                                const std::string& lot_in_time,
                                const std::string& lot_out_time)
{
    int retry = 3;

    while (retry > 0)
    {
        if (doSendParkInParkOutInfo(lot_no, lpn, lot_in_image, lot_out_image, lot_in_time, lot_out_time))
        {
            return true;
        }
        else
        {
            retry--;
        }
    }

    return false;
}

void Central::FnSetCentralStatus(bool status)
{
    centralStatus_.store(status);
}

bool Central::FnGetCentralStatus()
{
    return centralStatus_.load();
}
