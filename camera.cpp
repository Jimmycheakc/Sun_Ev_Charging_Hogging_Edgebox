#include <boost/asio.hpp>
#include <iostream>
#include <mutex>
#include "camera.h"

CameraServer* CameraServer::cameraServer_ = nullptr;
std::mutex CameraServer::mutex_;

CameraServer::CameraServer()
{

}

CameraServer* CameraServer::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (cameraServer_ == nullptr)
    {
        cameraServer_ = new CameraServer();
    }
    return cameraServer_;
}

void CameraServer::FnCameraServerInitialization(boost::asio::io_context& io_context, const std::string& address, unsigned short port)
{
    auto const address_ = boost::asio::ip::make_address(address);

    pCameraServer_ = std::make_shared<listener>(io_context, boost::asio::ip::tcp::endpoint{address_, port});
    pCameraServer_->run();
}