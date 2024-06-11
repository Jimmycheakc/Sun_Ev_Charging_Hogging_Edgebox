#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <ifaddrs.h>
#include <iomanip>
#include <string>
#include <sstream>
#include "common.h"
#include "version.h"
#include "log.h"

Common* Common::common_ = nullptr;
std::mutex Common::mutex_;

Common::Common()
{

}

Common* Common::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (common_ == nullptr)
    {
        common_ = new Common();
    }
    return common_;
}

std::string Common::FnGetFileName(const std::string& str)
{
    std::size_t found = str.find_last_of("/");
    if (found != std::string::npos)
    {
        return str.substr(found + 1);
    }
    return str;
}

void Common::FnLogExecutableInfo(const std::string& str)
{
    std::ostringstream info;
    info << "Start " << FnGetFileName(str) << " , version: " << SW_VERSION << " build:" << __DATE__ << __TIME__;
    Logger::getInstance()->FnLog(info.str(), "COMMON");
}

std::string Common::FnGetDateTime()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo = {};
    localtime_r(&timer, &timeinfo);
    
    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%d/%m/%y %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << " ";
    return oss.str();
}

std::string Common::FnGetDateTimeFormat_yymmdd()
{
    auto now = std::chrono::system_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo = {};
    localtime_r(&timer, &timeinfo);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%y%m%d");
    return oss.str();
}

std::string Common::FnGetDateTimeFormat_YYYY_MM_DD_HH_MM_SS()
{
    auto now = std::chrono::system_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo = {};
    localtime_r(&timer, &timeinfo);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Common::FnGetLocalIPAddress()
{
    std::string ipv4Address = "";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *tempAddr = NULL;
    int success = 0;
    // Retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        tempAddr = interfaces;
        while(tempAddr != NULL) {
            if(tempAddr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is "en0" which is the common name for Ethernet or Wi-Fi on macOS
                if(strcmp(tempAddr->ifa_name, "eth0") == 0) {
                    ipv4Address = inet_ntoa(((struct sockaddr_in*)tempAddr->ifa_addr)->sin_addr);
                    break; // Exit the loop after finding the IPv4 address
                }
            }
            tempAddr = tempAddr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return ipv4Address;
}

std::string Common::FnConvertImageToBase64String(const std::string& image_path)
{
    std::ifstream imageFile(image_path, std::ios::binary | std::ios::ate);

    if (!imageFile.is_open())
    {
        std::ostringstream oss;
        oss << "Error opening the image file :" << image_path;
        Logger::getInstance()->FnLog(oss.str(), "COMMON");
        return "";
    }

    std::streamsize size = imageFile.tellg();
    imageFile.seekg(0, std::ios::beg);

    std::string buffer(size, ' ');
    if (!imageFile.read(&buffer[0], size))
    {
        std::ostringstream oss;
        oss << "Error reading the image file :" << image_path;
        Logger::getInstance()->FnLog(oss.str(), "COMMON");
        return "";
    }

    imageFile.close();

    using namespace boost::archive::iterators;

    // Create a base64 encoder using Boost.Archive
    typedef base64_from_binary<transform_width<std::string::const_iterator, 6, 8>> base64_text;

    std::stringstream encoded;
    std::copy(base64_text(buffer.begin()), base64_text(buffer.end()), std::ostream_iterator<char>(encoded));

    // Append padding characters if needed
    size_t padding = (3 - buffer.size() % 3) % 3;
    encoded.write("===", padding);

    return encoded.str();
}