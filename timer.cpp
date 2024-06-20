#include <iostream>
#include "database.h"
#include "ini_parser.h"
#include "timer.h"

EvtTimer* EvtTimer::evtTimer_ = nullptr;
std::mutex EvtTimer::mutex_;


EvtTimer::EvtTimer()
{

}

EvtTimer* EvtTimer::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (evtTimer_ == nullptr)
    {
        evtTimer_ = new EvtTimer();
    }
    return evtTimer_;
}

void EvtTimer::FnTimerInitialization(boost::asio::io_context& io_context, boost::asio::strand<boost::asio::io_context::executor_type>& strand)
{
    pDeviceStatusUpdateTimer_ = std::make_shared<Timer>(io_context, strand);
    pHeartbeatCentralTimer_ = std::make_shared<Timer>(io_context, strand);
    pFirstParkingLotFilterTimer_ = std::make_shared<Timer>(io_context, strand);
    pSecondParkingLotFilterTimer_ = std::make_shared<Timer>(io_context, strand);
    pThirdParkingLotFilterTimer_ = std::make_shared<Timer>(io_context, strand);
}

void EvtTimer::onDeviceStatusUpdateTimerTimeout()
{
    Logger::getInstance()->FnLog(__func__, "TIMER");

    FnStartDeviceStatusUpdateTimer();
}

void EvtTimer::FnStartDeviceStatusUpdateTimer()
{
    pDeviceStatusUpdateTimer_->start(IniParser::getInstance()->FnGetTimerTimeoutForDeviceStatusUpdateToCentral(), 
                                std::bind(&EvtTimer::onDeviceStatusUpdateTimerTimeout, this));
}

void EvtTimer::onHeartbeatCentralTimerTimeout()
{
    Logger::getInstance()->FnLog(__func__, "TIMER");

    FnStartHeartbeatCentralTimer();
}

void EvtTimer::FnStartHeartbeatCentralTimer()
{
    pHeartbeatCentralTimer_->start(IniParser::getInstance()->FnGetTimerCentralHeartbeat(), 
                                std::bind(&EvtTimer::onHeartbeatCentralTimerTimeout, this));
}


// First parking lot filter timer
void EvtTimer::onFirstParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo)
{
    Logger::getInstance()->FnLog(__func__, "TIMER");

}

void EvtTimer::FnStartFirstParkingLotFilterTimer(const parking_lot_t& parkingLotInfo)
{
    std::unique_ptr<parking_lot_t> data = std::make_unique<parking_lot_t>(parkingLotInfo);

    pFirstParkingLotFilterTimer_->start(IniParser::getInstance()->FnGetTimerForFilteringSnapshot(), 
                                    std::function<void(std::unique_ptr<parking_lot_t>&&)>(
                                        std::bind(&EvtTimer::onFirstParkingLotFilterTimerTimeout, this, std::placeholders::_1)), 
                                        std::move(data));
}

void EvtTimer::FnStopFirstParkingLotFilterTimer()
{
    pFirstParkingLotFilterTimer_->stop();
}

bool EvtTimer::FnIsFirstParkingLotFilterTimerRunning()
{
    return pFirstParkingLotFilterTimer_->is_running();
}


// Second parking lot filter timer
void EvtTimer::onSecondParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo)
{
    Logger::getInstance()->FnLog(__func__, "TIMER");
}

void EvtTimer::FnStartSecondParkingLotFilterTimer(const parking_lot_t& parkingLotInfo)
{
    std::unique_ptr<parking_lot_t> data = std::make_unique<parking_lot_t>(parkingLotInfo);

    pSecondParkingLotFilterTimer_->start(IniParser::getInstance()->FnGetTimerForFilteringSnapshot(), 
                                    std::function<void(std::unique_ptr<parking_lot_t>&&)>(
                                        std::bind(&EvtTimer::onSecondParkingLotFilterTimerTimeout, this, std::placeholders::_1)), 
                                        std::move(data));
}

void EvtTimer::FnStopSecondParkingLotFilterTimer()
{
    pSecondParkingLotFilterTimer_->stop();
}

bool EvtTimer::FnIsSecondParkingLotFilterTimerRunning()
{
    return pSecondParkingLotFilterTimer_->is_running();
}


// Third parking lot filter timer
void EvtTimer::onThirdParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo)
{
    Logger::getInstance()->FnLog(__func__, "TIMER");

}

void EvtTimer::FnStartThirdParkingLotFilterTimer(const parking_lot_t& parkingLotInfo)
{
    std::unique_ptr<parking_lot_t> data = std::make_unique<parking_lot_t>(parkingLotInfo);

    pThirdParkingLotFilterTimer_->start(IniParser::getInstance()->FnGetTimerForFilteringSnapshot(), 
                                    std::function<void(std::unique_ptr<parking_lot_t>&&)>(
                                        std::bind(&EvtTimer::onThirdParkingLotFilterTimerTimeout, this, std::placeholders::_1)), 
                                        std::move(data));
}

void EvtTimer::FnStopThirdParkingLotFilterTimer()
{
    pThirdParkingLotFilterTimer_->stop();
}

bool EvtTimer::FnIsThirdParkingLotFilterTimerRunning()
{
    return pThirdParkingLotFilterTimer_->is_running();
}