#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ini_parser.h"

IniParser* IniParser::iniParser_ = nullptr;
std::mutex IniParser::mutex_;

IniParser::IniParser()
    : cameraIP_(""),
    centralIP_(""),
    centralServerPort_(0),
    parkingLotLocationCode_(""),
    timerForFilteringSnapshot_(0),
    timerTimeoutForDeviceStatusUpdateToCentral_(0),
    timerCentralHeartbeat_(0)
{

}

IniParser* IniParser::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (iniParser_ == nullptr)
    {
        iniParser_ = new IniParser();
    }
    return iniParser_;
}

bool IniParser::FnReadIniFile()
{
    bool ret = false;

    try
    {
        if (!(boost::filesystem::exists(INI_FILE_PATH)))
        {
            throw std::runtime_error("Ini file path: " + INI_FILE_PATH + " not exists.");
        }
        
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(INI_FILE_ABSOLUTE_PATH, pt);

        cameraIP_                                       = pt.get<std::string>("setting.cameraIP", "");
        centralIP_                                      = pt.get<std::string>("setting.centralIP", "");
        centralServerPort_                              = pt.get<int>("setting.centralServerPort");
        parkingLotLocationCode_                         = pt.get<std::string>("setting.parkingLotLocationCode");
        timerForFilteringSnapshot_                      = pt.get<int>("setting.timerForFilteringSnapshot");
        timerTimeoutForDeviceStatusUpdateToCentral_     = pt.get<int>("setting.timerTimeoutForDeviceStatusUpdateToCentral");
        timerCentralHeartbeat_                          = pt.get<int>("setting.timerCentralHeartbeat");

        ret = true;
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
        std::cerr << "Boost.Filesystem Exception during reading Ini file: " << e.what() << std::endl;
    }
    catch (const boost::property_tree::ptree_bad_path &e)
    {
        std::cerr << "Boost.PropertyTree Bad Path Exception during reading Ini file: " << e.what() << std::endl;
    }
    catch (const boost::property_tree::ptree_bad_data &e)
    {
        std::cerr << "Boost.PropertyTree Bad Data Exception during reading Ini file: " << e.what() << std::endl;
    }
    catch (const boost::property_tree::ptree_error &e)
    {
        std::cerr << "Boost.PropertyTree General Exception during reading Ini file: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during reading Ini file: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception during reading Ini file." << std::endl;
    }

    return ret;
}

std::string IniParser::FnGetCameraIP() const
{
    return cameraIP_;
}

std::string IniParser::FnGetCentralIP() const
{
    return centralIP_;
}

int IniParser::FnGetCentralServerPort() const
{
    return centralServerPort_;
}

std::string IniParser::FnGetParkingLotLocationCode() const
{
    return parkingLotLocationCode_;
}

int IniParser::FnGetTimerForFilteringSnapshot() const
{
    return timerForFilteringSnapshot_;
}

int IniParser::FnGetTimerTimeoutForDeviceStatusUpdateToCentral() const
{
    return timerTimeoutForDeviceStatusUpdateToCentral_;
}

int IniParser::FnGetTimerCentralHeartbeat() const
{
    return timerCentralHeartbeat_;
}
