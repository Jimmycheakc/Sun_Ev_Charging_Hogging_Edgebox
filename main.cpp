#include <iostream>
#include "central.h"
#include "common.h"
#include "database.h"
#include "ini_parser.h"
#include "log.h"


int main(int argc, char* argv[])
{
    if (IniParser::getInstance()->FnReadIniFile() == false)
    {
        std::exit(EXIT_FAILURE);
    }
    Logger::getInstance();
    Common::getInstance()->FnLogExecutableInfo(argv[0]);
    MariaDB::getInstance()->FnConnectMariaLocalDatabase();
    MariaDB::getInstance()->FnInsertEvLotStatusRecord(IniParser::getInstance()->FnGetParkingLotLocationCode(), Common::getInstance()->FnGetLocalIPAddress(), "1");
    int count = MariaDB::getInstance()->FnIsEvLotStatusTableEmpty();
    std::cout << "Count: " << count << std::endl;
    MariaDB::getInstance()->FnRemoveAllRecordFromEvLotStatusTable();
    count = MariaDB::getInstance()->FnIsEvLotStatusTableEmpty();
    std::cout << "Count: " << count << std::endl;

    MariaDB::parking_lot_t lot1 = {"", "", "", "", "", "", "", "", "", "", "", ""};
    MariaDB::getInstance()->FnInsertEvLotTransRecord(lot1);

    MariaDB::parking_lot_t lot = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    MariaDB::getInstance()->FnInsertEvLotTransRecord(lot);
    count = MariaDB::getInstance()->FnIsEvLotTransTableEmpty();
    std::cout << "Count: " << count << std::endl;
    MariaDB::getInstance()->FnRemoveAllRecordFromEvLotTransTable();
    count = MariaDB::getInstance()->FnIsEvLotTransTableEmpty();
    std::cout << "Count: " << count << std::endl;

    bool result = Central::getInstance()->FnSendHeartbeatUpdate();
    std::cout << "Central heartbeat result: " << result << std::endl;

    result = Central::getInstance()->FnSendDeviceStatusUpdate(Common::getInstance()->FnGetLocalIPAddress(), Central::ERROR_CODE_IPC);
    std::cout << "Device status send to central result: " << result << std::endl;

    result = Central::getInstance()->FnSendParkInParkOutInfo("165",
                                                            "SNN 4019 G", 
                                                            Common::getInstance()->FnConvertImageToBase64String("/home/root/ev_charging_hogging/Img_240411_213251.jpg"),
                                                            "",
                                                            Common::getInstance()->FnGetDateTimeFormat_YYYY_MM_DD_HH_MM_SS(),
                                                            "");

    return 0;
}