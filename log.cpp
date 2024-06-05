#include <sstream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include "common.h"
#include "log.h"

Logger* Logger::logger_ = nullptr;
std::mutex Logger::mutex_; 

Logger::Logger()
    : loggerName_("ev")
{
    FnCreateLogFile();
}

Logger::~Logger()
{
    spdlog::drop_all();
    spdlog::shutdown();
}

Logger* Logger::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (logger_ == nullptr)
    {
        logger_ = new Logger();
    }
    return logger_;
}

void Logger::FnCreateLogFile()
{
    try
    {
        const boost::filesystem::path dirPath(LOG_FILE_PATH);

        if (!(boost::filesystem::exists(dirPath)))
        {
            if (!(boost::filesystem::create_directories(dirPath)))
            {
                throw std::runtime_error("Failed to create directory: " + dirPath.string());
            }
        }

        std::string absoluteFilePath = LOG_FILE_PATH + "/ev_charging_hogging_" + Common::getInstance()->FnGetDateTimeFormate_yymmdd() + ".log";

        auto asyncFile = spdlog::basic_logger_mt<spdlog::async_factory>(loggerName_, absoluteFilePath);
        asyncFile->set_pattern("%v");
        asyncFile->set_level(spdlog::level::info);
        asyncFile->flush_on(spdlog::level::info);
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        std::cerr << "Boost.Filesystem Exception during creating log file: " << e.what() << std::endl;
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Spdlog initialization failed: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception during creating log file: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception during creating log file." << std::endl;
    }
}

void Logger::FnLog(std::string sMsg, std::string sOption)
{
    try
    {
        std::stringstream sLogMsg;

        sLogMsg << Common::getInstance()->FnGetDateTime();
        sLogMsg << std::setw(3) << std::setfill(' ');
        sLogMsg << std::setw(10) << std::left << sOption;
        sLogMsg << sMsg;

        std::string absoluteFilePath = LOG_FILE_PATH + "/ev_charging_hogging_" + Common::getInstance()->FnGetDateTimeFormate_yymmdd() + ".log";

        if (!(boost::filesystem::exists(absoluteFilePath)))
        {
            spdlog::drop(loggerName_);
            FnCreateLogFile();
        }

        auto logger = spdlog::get(loggerName_);
        if (logger)
        {
            logger->info(sLogMsg.str());
            logger->flush();
        }

        std::cout << sLogMsg.str() << std::endl;
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        std::cerr << "Boost.Filesystem Exception during writing log file: " << e.what() << std::endl;
    }
    catch (const spdlog::spdlog_ex &e)
    {
        std::cerr << "Spdlog failed during writing log file: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception during writing log file: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception during writing log file." << std::endl;
    }
}