#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "camera.h"
#include "central.h"
#include "common.h"
#include "database.h"
#include "ini_parser.h"
#include "log.h"
#include "structure.h"
#include "timer.h"

void worker(boost::asio::io_context& io_context)
{
    io_context.run();
}

int main(int argc, char* argv[])
{
    boost::asio::io_context io_context;
    boost::asio::strand<boost::asio::io_context::executor_type> strand(io_context.get_executor());

    // Create an io_context::work object to keep the io_context running
    boost::asio::io_context::work work(io_context);

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
    int id = MariaDB::getInstance()->FnGetLastInsertedIDFromEvLotStatusRecord();
    std::cout << "Id: " << id << std::endl;
    MariaDB::getInstance()->FnRemoveAllRecordFromEvLotStatusTable();
    count = MariaDB::getInstance()->FnIsEvLotStatusTableEmpty();
    std::cout << "Count: " << count << std::endl;

    parking_lot_t lot1 = {"", "", "", "", "", "", "", "", "", "", ""};
    MariaDB::getInstance()->FnInsertEvLotTransRecord(lot1);
    id = MariaDB::getInstance()->FnGetLastInsertedIDFromEvLotTransRecord();
    std::cout << "Id: " << id << std::endl;

    parking_lot_t lot = {"1", "2", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    MariaDB::getInstance()->FnInsertEvLotTransRecord(lot);
    id = MariaDB::getInstance()->FnGetLastInsertedIDFromEvLotTransRecord();
    std::cout << "Id: " << id << std::endl;
    count = MariaDB::getInstance()->FnIsEvLotTransTableEmpty();
    std::cout << "Count: " << count << std::endl;
    MariaDB::getInstance()->FnRemoveAllRecordFromEvLotTransTable();
    count = MariaDB::getInstance()->FnIsEvLotTransTableEmpty();
    std::cout << "Count: " << count << std::endl;

    Central::getInstance()->FnCentralInitialization(io_context, strand);
    Central::getInstance()->FnSendDeviceStatusUpdate(Common::getInstance()->FnGetLocalIPAddress(), Central::ERROR_CODE_IPC);
    Central::getInstance()->FnSendHeartbeatUpdate();
    Central::getInstance()->FnSendParkInParkOutInfo("165",
                                                    "SNN 4019 G", 
                                                    Common::getInstance()->FnConvertImageToBase64String("/home/root/ev_charging_hogging/Img_240411_213251.jpg"),
                                                    "",
                                                    Common::getInstance()->FnGetDateTimeFormat_YYYY_MM_DD_HH_MM_SS(),
                                                    "");

    EvtTimer::getInstance()->FnTimerInitialization(io_context, strand);
    EvtTimer::getInstance()->FnStartDeviceStatusUpdateTimer();
    EvtTimer::getInstance()->FnStartHeartbeatCentralTimer();
    parking_lot_t lotFirst = {"1", "WWW1111", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    EvtTimer::getInstance()->FnStartFirstParkingLotFilterTimer(lotFirst);
    
    parking_lot_t lotSecond = {"1", "WWW2222", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    EvtTimer::getInstance()->FnStartSecondParkingLotFilterTimer(lotSecond);

    parking_lot_t lotThird = {"1", "WWW3333", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    EvtTimer::getInstance()->FnStartThirdParkingLotFilterTimer(lotThird);

    CameraServer::getInstance()->FnCameraServerInitialization(io_context, "192.168.2.150", 9999);

    boost::thread_group threads;
    for (std::size_t i = 0; i < 6; i++)
    {
        threads.create_thread(std::bind(&worker, std::ref(io_context)));
    }


    threads.join_all();

    return 0;
}