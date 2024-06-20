#pragma once

#include <iostream>
#include <string>

typedef struct
{
    std::string location_code;
    std::string lot_no;
    std::string lpn;
    std::string lot_in_image_path;
    std::string lot_out_image_path;
    std::string lot_in_dt;
    std::string lot_out_dt;
    std::string add_dt;
    std::string update_dt;
    std::string lot_in_central_sent_dt;
    std::string lot_out_central_sent_dt;
} parking_lot_t;