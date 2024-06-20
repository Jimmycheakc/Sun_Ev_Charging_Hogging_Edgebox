#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <iostream>
#include <functional>
#include <memory>
#include "log.h"
#include "structure.h"

class Timer : public std::enable_shared_from_this<Timer>
{
public:
    Timer(boost::asio::io_context& io_context, boost::asio::strand<boost::asio::io_context::executor_type>& strand)
        : timer_(io_context), strand_(strand), isTimerRunning_(false)
    {
        
    }

    void start(int seconds, std::function<void()> callback)
    {
        if (!isTimerRunning_.load())
        {
            isTimerRunning_.store(true);
            timer_.expires_from_now(boost::posix_time::seconds(seconds));
            auto self = shared_from_this();
            timer_.async_wait(boost::asio::bind_executor(strand_, [self, callback](const boost::system::error_code& ec) {
                self->isTimerRunning_.store(false);
                if (!ec)
                {
                    callback();
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    std::ostringstream oss;
                    oss << "Boost.Asio Exception :" << ec.message();
                    Logger::getInstance()->FnLog(oss.str(), "TIMER");
                }
            }));
        }
    }

    template<class Arg>
    void start(int seconds, std::function<void(Arg&&)> callback, Arg&& arg)
    {
        if (!isTimerRunning_.load())
        {
            isTimerRunning_.store(true);
            timer_.expires_from_now(boost::posix_time::seconds(seconds));
            auto self = shared_from_this();
            timer_.async_wait(boost::asio::bind_executor(strand_, [self, callback, arg = std::move(arg)](const boost::system::error_code& ec) mutable {
                self->isTimerRunning_.store(false);
                if (!ec)
                {
                    callback(std::move(arg));
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                    std::ostringstream oss;
                    oss << "Boost.Asio Exception :" << ec.message();
                    Logger::getInstance()->FnLog(oss.str(), "TIMER");
                }
            }));
        }
    }

    bool is_running() const
    {
        return isTimerRunning_.load();
    }

    void stop()
    {
        if (isTimerRunning_.load())
        {
            timer_.cancel();
            isTimerRunning_.store(false);
        }
    }

private:
    boost::asio::deadline_timer timer_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::atomic<bool> isTimerRunning_;
};

class EvtTimer
{
public:
    static EvtTimer* getInstance();
    void FnTimerInitialization(boost::asio::io_context& io_context, boost::asio::strand<boost::asio::io_context::executor_type>& strand);
    void FnStartDeviceStatusUpdateTimer();
    void FnStartHeartbeatCentralTimer();

    void FnStartFirstParkingLotFilterTimer(const parking_lot_t& parkingLotInfo);
    void FnStopFirstParkingLotFilterTimer();
    bool FnIsFirstParkingLotFilterTimerRunning();
    void FnStartSecondParkingLotFilterTimer(const parking_lot_t& parkingLotInfo);
    void FnStopSecondParkingLotFilterTimer();
    bool FnIsSecondParkingLotFilterTimerRunning();
    void FnStartThirdParkingLotFilterTimer(const parking_lot_t& parkingLotInfo);
    void FnStopThirdParkingLotFilterTimer();
    bool FnIsThirdParkingLotFilterTimerRunning();

    /*
     * Singleton EvtTimer cannot be cloneable
     */
    EvtTimer(EvtTimer& evtTimer_) = delete;

    /*
     * Singleton EvtTimer cannot be assignable
     */
    void operator=(const EvtTimer&) = delete;

private:
    static EvtTimer* evtTimer_;
    static std::mutex mutex_;
    EvtTimer();

    std::shared_ptr<Timer> pDeviceStatusUpdateTimer_;
    std::shared_ptr<Timer> pHeartbeatCentralTimer_;
    std::shared_ptr<Timer> pFirstParkingLotFilterTimer_;
    std::shared_ptr<Timer> pSecondParkingLotFilterTimer_;
    std::shared_ptr<Timer> pThirdParkingLotFilterTimer_;

    void onDeviceStatusUpdateTimerTimeout();
    void onHeartbeatCentralTimerTimeout();
    void onFirstParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo);
    void onSecondParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo);
    void onThirdParkingLotFilterTimerTimeout(std::unique_ptr<parking_lot_t> lotInfo);
};