#pragma once

#include <iostream>
#include <mutex>

class IniParser
{
public:
    const std::string INI_FILE_PATH = "/home/root/ev_charging_hogging/Ini";
    const std::string INI_FILE_ABSOLUTE_PATH = "/home/root/ev_charging_hogging/Ini/configuration.ini";

    static IniParser* getInstance();
    bool FnReadIniFile();

    std::string FnGetCameraIP() const;
    std::string FnGetCentralIP() const;
    int FnGetCentralServerPort() const;
    std::string FnGetParkingLotLocationCode() const;
    int FnGetTimerForFilteringSnapshot() const;
    int FnGetTimerTimeoutForDeviceStatusUpdateToCentral() const;
    int FnGetTimerCentralHeartbeat() const;

    /*
     * Singleton IniParser should not be cloneable
     */
    IniParser(IniParser &iniparser) = delete;

    /*
     * Singleton IniParser should not be assignable
     */
    void operator=(const IniParser&) = delete;

private:
    static IniParser* iniParser_;
    static std::mutex mutex_;
    IniParser();

    std::string cameraIP_;
    std::string centralIP_;
    int centralServerPort_;
    std::string parkingLotLocationCode_;
    int timerForFilteringSnapshot_;
    int timerTimeoutForDeviceStatusUpdateToCentral_;
    int timerCentralHeartbeat_;
};