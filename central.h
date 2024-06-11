#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>

class Central
{

public:
    static const std::string ERROR_CODE_RECOVERED;
    static const std::string ERROR_CODE_CAMERA;
    static const std::string ERROR_CODE_IPC;

    const std::string USERNAME = "testuser";
    const std::string PASSWORD = "testpassword";

    static Central* getInstance();

    bool FnSendHeartbeatUpdate();
    bool FnSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code);
    bool FnSendParkInParkOutInfo(const std::string& lot_no,
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
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;
    std::string centralServerIp_;
    int centralServerPort_;

    bool doSendHeartbeatUpdate();
    bool doSendDeviceStatusUpdate(const std::string& device_ip, const std::string& error_code);
    bool doSendParkInParkOutInfo(const std::string& lot_no,
                                const std::string& lpn,
                                const std::string& lot_in_image,
                                const std::string& lot_out_image,
                                const std::string& lot_in_time,
                                const std::string& lot_out_time);
};