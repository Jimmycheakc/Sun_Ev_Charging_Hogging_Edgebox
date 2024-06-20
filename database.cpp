#include "database.h"
#include "log.h"

MariaDB* MariaDB::mariaDb_ = nullptr;
std::mutex MariaDB::mutex_;

MariaDB::MariaDB()
    : databaseStatus_(false),
    databaseRecoveryFlag_(false)
{

}

MariaDB* MariaDB::getInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (mariaDb_ == nullptr)
    {
        mariaDb_ = new MariaDB();
    }
    return mariaDb_;
}

void MariaDB::FnConnectMariaLocalDatabase()
{
    if (!mariaDatabase_)
    {
        mariaDatabase_ = std::make_unique<OdbcDatabase>();
    }

    if (mariaDatabase_)
    {
        if (mariaDatabase_->connect(DB_DRIVER, DB_SERVER, DB_PORT, DB_NAME, DB_USERNAME, DB_PASSWORD, 2))
        {
            Logger::getInstance()->FnLog("Successful connected to ev_charging_database.", "DB");
            FnSetDatabaseStatus(true);
        }
        else
        {
            Logger::getInstance()->FnLog("Failed to connect to ev_charging_database.", "DB");
            FnSetDatabaseStatus(false);
        }
    }
}

bool MariaDB::FnIsConnected()
{
    bool ret = false;

    if (mariaDatabase_)
    {
        ret = mariaDatabase_->is_connected();
    }

    return ret; 
}

void MariaDB::FnSetDatabaseStatus(bool status)
{
    databaseStatus_ = status;
}

bool MariaDB::FnGetDatabaseStatus()
{
    return databaseStatus_;
}

void MariaDB::FnSetDatabaseRecoveryFlag(bool flag)
{
    databaseRecoveryFlag_ = flag;
}

bool MariaDB::FnGetDatabaseRecoveryFlag()
{
    return databaseRecoveryFlag_;
}

bool MariaDB::FnReconnectMariaLocalDatabase()
{
    if (!FnIsConnected())
    {
        Logger::getInstance()->FnLog("Attempting to reconnect to ev_charging_database...", "DB");

        // Reset the unique_ptr to release the current connection
        mariaDatabase_.reset();

        // Create a new instance and attempt reconnect
        mariaDatabase_ = std::make_unique<OdbcDatabase>();

        if (mariaDatabase_->connect(DB_DRIVER, DB_SERVER, DB_PORT, DB_NAME, DB_USERNAME, DB_PASSWORD, 2))
        {
            Logger::getInstance()->FnLog("Reconnected to ev_charging_database successfully.", "DB");
            FnSetDatabaseStatus(true);
            FnSetDatabaseRecoveryFlag(false);
            return true;
        }
        else
        {
            Logger::getInstance()->FnLog("Failed to reconnect ev_charging_database.", "DB");
            FnSetDatabaseStatus(false);
            FnSetDatabaseRecoveryFlag(true);
            return false;
        }
    }

    return true;
}

bool MariaDB::FnInsertEvLotStatusRecord(const std::string& carpark_code, const std::string& device_ip, const std::string& error_code)
{
    bool ret = false;
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return ret;
    }

    std::ostringstream query;
    query << "INSERT INTO tbl_ev_lot_status (location_code, device_ip, error_code) VALUES (";
    
    if (carpark_code.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << carpark_code << "', ";
    }

    if (device_ip.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << device_ip << "', ";
    }

    if (error_code.empty())
    {
        query << "NULL)";
    }
    else
    {
        query << "'" << error_code << "')";
    }

    bool result = mariaDatabase_->execute_non_query(query.str());

    if (result)
    {
        Logger::getInstance()->FnLog(query.str(), "DB");
        ret = true;
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to execute insert query: " + query.str(), "DB");
        ret = false;
    }

    return ret;
}

int MariaDB::FnGetLastInsertedIDFromEvLotStatusRecord()
{
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return -1;
    }

    return mariaDatabase_->get_last_inserted_id("tbl_ev_lot_status");
}

bool MariaDB::FnIsEvLotStatusTableEmpty()
{
    bool ret = false;
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return ret;
    }

    std::string query = "SELECT COUNT(*) FROM tbl_ev_lot_status";
    int result = mariaDatabase_->select_count(query);

    if (result == 0)
    {
        std::stringstream ss;
        ss << query << ", result: " << result;
        Logger::getInstance()->FnLog(ss.str(), "DB");
        ret = true;
    }
    else if (result > 0)
    {
        std::stringstream ss;
        ss << query << ", result: " << result;
        Logger::getInstance()->FnLog(ss.str(), "DB");
        ret = false;
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to select count query: " + query, "DB");
        ret = false;
    }

    return ret;
}

void MariaDB::FnRemoveAllRecordFromEvLotStatusTable()
{
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return;
    }

    std::string query = "DELETE FROM tbl_ev_lot_status";
    bool result = mariaDatabase_->execute_non_query(query);

    if (result)
    {
        Logger::getInstance()->FnLog(query, "DB");
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to delete all query: " + query, "DB");
    }
}

bool MariaDB::FnInsertEvLotTransRecord(const parking_lot_t& lot)
{
    bool ret = false;
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return ret;
    }

    std::ostringstream query;
    query << "INSERT INTO tbl_ev_lot_trans (location_code, lot_no, lpn, lot_in_image, lot_out_image, lot_in_dt, lot_out_dt, add_dt, update_dt, lot_in_central_sent_dt, lot_out_central_sent_dt) VALUES (";
    
    if (lot.location_code.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.location_code << "', ";
    }

    if (lot.lot_no.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_no << "', ";
    }

    if (lot.lpn.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lpn << "', ";
    }

    if (lot.lot_in_image_path.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_in_image_path << "', ";
    }

    if (lot.lot_out_image_path.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_out_image_path << "', ";
    }

    if (lot.lot_in_dt.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_in_dt << "', ";
    }

    if (lot.lot_out_dt.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_out_dt << "', ";
    }

    if (lot.add_dt.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.add_dt << "', ";
    }

    if (lot.update_dt.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.update_dt << "', ";
    }

    if (lot.lot_in_central_sent_dt.empty())
    {
        query << "NULL, ";
    }
    else
    {
        query << "'" << lot.lot_in_central_sent_dt << "', ";
    }

    if (lot.lot_out_central_sent_dt.empty())
    {
        query << "NULL)";
    }
    else
    {
        query << "'" << lot.lot_out_central_sent_dt << "')";
    }

    bool result = mariaDatabase_->execute_non_query(query.str());

    if (result)
    {
        Logger::getInstance()->FnLog(query.str(), "DB");
        ret = true;
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to execute insert query: " + query.str(), "DB");
        ret = false;
    }

    return ret;
}

int MariaDB::FnGetLastInsertedIDFromEvLotTransRecord()
{
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return -1;
    }

    return mariaDatabase_->get_last_inserted_id("tbl_ev_lot_trans");
}

bool MariaDB::FnIsEvLotTransTableEmpty()
{
    bool ret = false;
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return ret;
    }

    std::string query = "SELECT COUNT(*) FROM tbl_ev_lot_trans";
    int result = mariaDatabase_->select_count(query);

    if (result == 0)
    {
        std::stringstream ss;
        ss << query << ", result: " << result;
        Logger::getInstance()->FnLog(ss.str(), "DB");
        ret = true;
    }
    else if (result > 0)
    {
        std::stringstream ss;
        ss << query << ", result: " << result;
        Logger::getInstance()->FnLog(ss.str(), "DB");
        ret = false;
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to select count query: " + query, "DB");
        ret = false;
    }

    return ret;
}

void MariaDB::FnRemoveAllRecordFromEvLotTransTable()
{
    Logger::getInstance()->FnLog(__func__, "DB");

    if (!FnIsConnected())
    {
        FnSetDatabaseStatus(false);
        Logger::getInstance()->FnLog("Database is not connected.", "DB");
        return;
    }

    std::string query = "DELETE FROM tbl_ev_lot_trans";
    bool result = mariaDatabase_->execute_non_query(query);

    if (result)
    {
        Logger::getInstance()->FnLog(query, "DB");
    }
    else
    {
        Logger::getInstance()->FnLog("Failed to delete all query: " + query, "DB");
    }
}