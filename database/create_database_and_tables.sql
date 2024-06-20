-- Create_database_and_table_sceipt.sql

-- Create a new database
CREATE DATABASE IF NOT EXISTS ev_charging_database;
USE ev_charging_database;

-- Create a table in the database
CREATE TABLE IF NOT EXISTS tbl_ev_lot_trans (
    id INT AUTO_INCREMENT PRIMARY KEY,
    location_code VARCHAR(10),
    lot_no VARCHAR(10),
    lpn VARCHAR(10),
    lot_in_image VARCHAR(200),
    lot_out_image VARCHAR(200),
    lot_in_dt DATETIME,
    lot_out_dt DATETIME,
    add_dt DATETIME,
    update_dt DATETIME,
    lot_in_central_sent_dt DATETIME,
    lot_out_central_sent_dt DATETIME
);

-- Create a table in the database
CREATE TABLE IF NOT EXISTS tbl_ev_lot_status (
    id INT AUTO_INCREMENT PRIMARY KEY,
    location_code VARCHAR(10),
    device_ip VARCHAR(20),
    error_code VARCHAR(10),
    central_sent_dt DATETIME
);

-- Create a new user and grant privileges
CREATE USER IF NOT EXISTS 'evcharging'@'localhost' IDENTIFIED BY 'SJ2001';
GRANT ALL PRIVILEGES ON ev_charging_database.* TO 'evcharging'@'localhost';
FLUSH PRIVILEGES;