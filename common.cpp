#include <chrono>
#include <ctime>
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
    Logger::getInstance()->FnLog(info.str(), "COM");
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

std::string Common::FnGetDateTimeFormate_yymmdd()
{
    auto now = std::chrono::system_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo = {};
    localtime_r(&timer, &timeinfo);

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%y%m%d");
    return oss.str();
}