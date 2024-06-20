#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
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
    : centralStatus_(false)
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

void Central::FnCentralInitialization(boost::asio::io_context& io_context, boost::asio::strand<boost::asio::io_context::executor_type>& strand)
{
    pSendDeviceStatusSession_ = std::make_shared<httpClientSession>(io_context, strand, std::bind(&Central::onSendDeviceStatusUpdateCallbackHandler, this, std::placeholders::_1, std::placeholders::_2));
    pSendHeartbeatSession_ = std::make_shared<httpClientSession>(io_context, strand, std::bind(&Central::onSendHeartbeatUpdateCallbackHandler, this, std::placeholders::_1, std::placeholders::_2));
    pSendParkInParkOutSession_ = std::make_shared<httpClientSession>(io_context, strand, std::bind(&Central::onSendParkInParkOutCallbackHandler, this, std::placeholders::_1, std::placeholders::_2));
}

void Central::onSendHeartbeatUpdateCallbackHandler(boost::beast::error_code ec, const std::string& msg)
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

    if (!ec)
    {
        Logger::getInstance()->FnLog("Send heart beat successfully.", "CENTRAL");
    }
    else
    {
        std::ostringstream oss;
        oss << msg << " :" << ec.message();
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");
    }
}

void Central::FnSendHeartbeatUpdate()
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

    boost::json::object jsonObject;
    jsonObject["username"] = USERNAME;
    jsonObject["password"] = PASSWORD;
    jsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
    jsonObject["heartbeat_dt"] = Common::getInstance()->FnGetDateTimeFormat_YYYY_MM_DD_HH_MM_SS();
    jsonObject["msg"] = "Heartbeat Update";

    std::string body = boost::json::serialize(jsonObject);

    Logger::getInstance()->FnLog("Request JSON Body :" + body, "CENTRAL");
    pSendHeartbeatSession_->run(IniParser::getInstance()->FnGetCentralIP(), std::to_string(IniParser::getInstance()->FnGetCentralServerPort()), "/HeartBeat", 11, body);
}

void Central::onSendDeviceStatusUpdateCallbackHandler(boost::beast::error_code ec, const std::string& msg)
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

    if (!ec)
    {
        Logger::getInstance()->FnLog("Send device status successfully.", "CENTRAL");
    }
    else
    {
        std::ostringstream oss;
        oss << msg << " :" << ec.message();
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");
    }
}

void Central::FnSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code)
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

    boost::json::object jsonObject;
    jsonObject["username"] = USERNAME;
    jsonObject["password"] = PASSWORD;
    jsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
    jsonObject["device_ip"] = device_ip;
    jsonObject["error_code"] = error_code;

    std::string body = boost::json::serialize(jsonObject);

    Logger::getInstance()->FnLog("Request JSON Body :" + body, "CENTRAL");
    pSendDeviceStatusSession_->run(IniParser::getInstance()->FnGetCentralIP(), std::to_string(IniParser::getInstance()->FnGetCentralServerPort()), "/DeviceStatus", 11, body);
}

void Central::onSendParkInParkOutCallbackHandler(boost::beast::error_code ec, const std::string& msg)
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

    if (!ec)
    {
        Logger::getInstance()->FnLog("Send park in park out info successfully.", "CENTRAL");
    }
    else
    {
        std::ostringstream oss;
        oss << msg << " :" << ec.message();
        Logger::getInstance()->FnLog(oss.str(), "CENTRAL");
    }
}

void Central::FnSendParkInParkOutInfo(const std::string& lot_no,
                                const std::string& lpn,
                                const std::string& lot_in_image,
                                const std::string& lot_out_image,
                                const std::string& lot_in_time,
                                const std::string& lot_out_time)
{
    Logger::getInstance()->FnLog(__func__, "CENTRAL");

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

    // For logging only
    boost::json::object logJsonObject;
    logJsonObject["username"] = USERNAME;
    logJsonObject["password"] = PASSWORD;
    logJsonObject["carpark_code"] = IniParser::getInstance()->FnGetParkingLotLocationCode();
    logJsonObject["lot_no"] = lot_no;
    logJsonObject["lpn"] = lpn;
    logJsonObject["lot_in_image"] = lot_in_image.empty() ? "" : "Lot In Image";
    logJsonObject["lot_out_image"] = lot_out_image.empty() ? "" : "Lot Out Image";
    logJsonObject["lot_in_time"] = lot_in_time;
    logJsonObject["lot_out_time"] = lot_out_time;

    std::string logBody = boost::json::serialize(logJsonObject);
    Logger::getInstance()->FnLog("Request JSON Body :" + logBody, "CENTRAL");
    // End for Logging

    pSendParkInParkOutSession_->run(IniParser::getInstance()->FnGetCentralIP(), std::to_string(IniParser::getInstance()->FnGetCentralServerPort()), "/ParkInOut", 11, body);
}

void Central::FnSetCentralStatus(bool status)
{
    centralStatus_.store(status);
}

bool Central::FnGetCentralStatus()
{
    return centralStatus_.load();
}
