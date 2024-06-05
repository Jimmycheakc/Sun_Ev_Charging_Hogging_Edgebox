#pragma once

#include <iostream>
#include <string>
#include <mutex>

class Common
{
public:
    static Common* getInstance();
    std::string FnGetFileName(const std::string& str);
    void FnLogExecutableInfo(const std::string& str);
    std::string FnGetDateTime();
    std::string FnGetDateTimeFormate_yymmdd();

    /*
     * Singleton Common should not be cloneable
     */
    Common(Common& common) = delete;

    /*
     * Singleton Common should not be assignable
     */
    void operator=(const Common&) = delete;

private:
    static Common* common_;
    static std::mutex mutex_;
    Common();
};