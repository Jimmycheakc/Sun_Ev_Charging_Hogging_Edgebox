#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include "log.h"
#include "structure.h"

class OdbcDatabase
{
public:
    OdbcDatabase() : hEnv_(NULL), hDbc_(NULL), hStmt_(NULL), connected_(false) {}

    ~OdbcDatabase()
    {
        disconnect();
    }

    bool connect(const std::string& driver, const std::string& server, const std::string& port, 
        const std::string& database, const std::string& user, const std::string& pass, int connTimeout)
    {
        disconnect(); // Ensure any previous connections are closed

        SQLRETURN ret;
        ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv_);
        if (!check_error(ret, SQL_HANDLE_ENV, hEnv_, "SQLAllocHandle ENV"))
        {
            return false;
        }

        ret = SQLSetEnvAttr(hEnv_, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
        if (!check_error(ret, SQL_HANDLE_ENV, hEnv_, "SQLSetEnvAttr"))
        {
            return false;
        }

        ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv_, &hDbc_);
        if (!check_error(ret, SQL_HANDLE_ENV, hEnv_, "SQLAllocHandle DBC"))
        {
            return false;
        }

        ret = SQLSetConnectAttr(hDbc_, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)(intptr_t)connTimeout, 0);
        if (!check_error(ret, SQL_HANDLE_DBC, hDbc_, "SQLSetConnectAttr CONNECTION_TIMEOUT"))
        {
            return false;
        }

        std::string connStr = "DRIVER={" + driver + "};SERVER=" + server + ";PORT=" + port + ";DATABASE=" + database + ";UID=" + user + ";PWD=" + pass + ";";
        ret = SQLDriverConnect(hDbc_, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
        if (!check_error(ret, SQL_HANDLE_DBC, hDbc_, "SQLDriverConnect"))
        {
            return false;
        }

        connected_ = true;
        return true;
    }

    void disconnect()
    {
        if (hStmt_)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
        }

        if (hDbc_)
        {
            SQLDisconnect(hDbc_);
            SQLFreeHandle(SQL_HANDLE_DBC, hDbc_);
            hDbc_ = NULL;
        }

        if (hEnv_)
        {
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv_);
            hEnv_ =NULL;
        }
        connected_ = false;
    }

    bool is_connected()
    {
        if (!connected_)
        {
            return false;
        }

        SQLINTEGER connectionStatus = 0;
        SQLRETURN ret = SQLGetConnectAttr(hDbc_, SQL_ATTR_CONNECTION_DEAD, &connectionStatus, 0, NULL);
        if (!check_error(ret, SQL_HANDLE_DBC, hDbc_, "SQLGetConnectAttr SQL_ATTR_CONNECTION_DEAD"))
        {
            return false;
        }

        return (connectionStatus == SQL_CD_FALSE);
    }

    bool execute_non_query(const std::string& query)
    {
        if (!connected_)
        {
            return false;
        }

        SQLRETURN ret;

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt_);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLAllocHandle STMT"))
        {
            hStmt_ = NULL;
            return false;
        }

        ret = SQLExecDirect(hStmt_, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLExecDirect"))
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
        hStmt_ = NULL;

        return true;
    }

    int get_last_inserted_id(const std::string& tableName)
    {
        if (!connected_)
        {
            return -1;
        }

        SQLRETURN ret;
        SQLINTEGER lastInsertedID = 0;

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt_);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLAllocHandle STMT"))
        {
            hStmt_ = NULL;
            return -1;
        }

        std::string query = "SELECT LAST_INSERT_ID() FROM " + tableName;
        ret = SQLExecDirect(hStmt_, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SELECT LAST_INSERT_ID()"))
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
            return -1;
        }

        ret = SQLFetch(hStmt_);
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
        {
            ret = SQLGetData(hStmt_, 1, SQL_C_LONG, &lastInsertedID, 0, NULL);
            if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLGetData"))
            {
                SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
                hStmt_ = NULL;
                return -1;
            }
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
        hStmt_ = NULL;

        return static_cast<int>(lastInsertedID);
    }

    int select_count(const std::string& query)
    {
        int count = -1;
        if (!connected_)
        {
            return count;
        }

        SQLRETURN ret;
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt_);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLAllocHandle STMT"))
        {
            hStmt_ = NULL;
            return count;
        }

        ret = SQLExecDirect(hStmt_, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLExecDirect"))
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
            return count;
        }

        if (SQLFetch(hStmt_) == SQL_SUCCESS)
        {
            SQLINTEGER num;
            ret = SQLGetData(hStmt_, 1, SQL_C_SLONG, &num, sizeof(num), NULL);
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
            {
                count = num;
            }
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
        hStmt_ = NULL;

        return count;
    }

    std::vector<std::vector<std::string>> select(const std::string& query)
    {
        std::vector<std::vector<std::string>> results;
        if (!connected_)
        {
            return results;
        }

        SQLRETURN ret;
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc_, &hStmt_);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLAllocHandle STMT"))
        {
            hStmt_ = NULL;
            return results;
        }

        ret = SQLExecDirect(hStmt_, (SQLCHAR*)query.c_str(), SQL_NTS);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLExecDirect"))
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
            return results;
        }

        SQLSMALLINT columns;
        ret = SQLNumResultCols(hStmt_, &columns);
        if (!check_error(ret, SQL_HANDLE_STMT, hStmt_, "SQLNumResultCols"))
        {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
            hStmt_ = NULL;
            return results;
        }

        while (SQLFetch(hStmt_) == SQL_SUCCESS)
        {
            std::vector<std::string> row;
            for (SQLSMALLINT i = 1; i <= columns; i++)
            {
                SQLCHAR buffer[256];
                ret = SQLGetData(hStmt_, i, SQL_C_CHAR, buffer, sizeof(buffer), NULL);
                if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
                {
                    row.push_back((char*)buffer);
                }
                else
                {
                    row.push_back("");
                }
            }
            results.push_back(row);
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt_);
        hStmt_ = NULL;

        return results;
    }

private:
    SQLHENV hEnv_;
    SQLHDBC hDbc_;
    SQLHSTMT hStmt_;
    bool connected_;

    bool check_error(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::string& msg)
    {
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
        {
            SQLCHAR sqlState[1024];
            SQLCHAR message[1024];
            if (SQLGetDiagRec(handleType, handle, 1, sqlState, NULL, message, 1024, NULL))
            {
                std::stringstream ss;
                ss << msg << " - Error: " << message << " (SQL State: " << sqlState << ")";
                Logger::getInstance()->FnLog(ss.str(), "ODBC");
            }
            else
            {
                std::stringstream ss;
                ss << msg << " - Unknown error.";
                Logger::getInstance()->FnLog(ss.str(), "ODBC");
            }
            return false;
        }
        return true;
    }

};

class MariaDB
{

public:
    const std::string DB_DRIVER      = "MariaDB ODBC 3.0 Driver";
    const std::string DB_SERVER      = "localhost";
    const std::string DB_PORT        = "3306";
    const std::string DB_NAME        = "ev_charging_database";
    const std::string DB_USERNAME    = "evcharging";
    const std::string DB_PASSWORD    = "SJ2001";

    static MariaDB* getInstance();
    void FnConnectMariaLocalDatabase();
    bool FnIsConnected();
    void FnSetDatabaseStatus(bool status);
    bool FnGetDatabaseStatus();
    void FnSetDatabaseRecoveryFlag(bool flag);
    bool FnGetDatabaseRecoveryFlag();
    bool FnReconnectMariaLocalDatabase();

    // Table --> tbl_ev_lot_status
    bool FnInsertEvLotStatusRecord(const std::string& carpark_code, const std::string& device_ip, const std::string& error_code);
    int FnGetLastInsertedIDFromEvLotStatusRecord();
    bool FnIsEvLotStatusTableEmpty();
    void FnRemoveAllRecordFromEvLotStatusTable();

    // Table --> tbl_ev_lot_trans
    bool FnInsertEvLotTransRecord(const parking_lot_t& lot);
    int FnGetLastInsertedIDFromEvLotTransRecord();
    bool FnIsEvLotTransTableEmpty();
    void FnRemoveAllRecordFromEvLotTransTable();

    /*
     * Singleton MariaDB cannot be cloneable
     */
    MariaDB(MariaDB& mariadb) = delete;

    /*
     * Singleton MariaDB cannot be assignable
     */
    void operator=(const MariaDB &) = delete;

private:
    static MariaDB* mariaDb_;
    static std::mutex mutex_;
    MariaDB();

    std::unique_ptr<OdbcDatabase> mariaDatabase_;
    bool databaseStatus_;
    bool databaseRecoveryFlag_;
};