#pragma once

#include <iostream>
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

class Logger
{
public:
    const std::string LOG_FILE_PATH = "/home/root/ev_charging_hogging/Log";

    static Logger* getInstance();
    void FnCreateLogFile();
    void FnLog(std::string sMsg, std::string sOption);

    /**
     * Singleton Logger should not be cloneable.
     */
    Logger(Logger &logger) = delete;

    /*
     * Singleton Logger should not be assignable.
     */
    void operator=(const Logger&) = delete;

private:
    static Logger* logger_;
    static std::mutex mutex_;
    std::string loggerName_;
    Logger();
    ~Logger();
};