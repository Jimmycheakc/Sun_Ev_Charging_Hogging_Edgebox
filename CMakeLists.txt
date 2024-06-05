cmake_minimum_required(VERSION 3.12)

project(EvChargingHogging)

# Find ODBC
find_package(ODBC REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Set the root directory for the target environment
set(CMAKE_FIND_ROOT_PATH ../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux)

# Specify the sysroot directory
set(CMAKE_SYSROOT ../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux)

add_definitions(-DFMT_HEADER_ONLY)

# Add include directories
include_directories(../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux/usr/include)
include_directories(../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux/usr/include/spdlog)
include_directories(../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux/usr/include/boost)

# Add link directories
link_directories(../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux/usr/local/lib)
link_directories(../../../SDK_2022.1/sysroots/cortexa72-cortexa53-xilinx-linux/usr/lib)

# Lisy your source files
set(SOURCE_FILES
    ini_parser.cpp
    common.cpp
    log.cpp
    main.cpp
)

# Copy ev charging hogging configuration.ini to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configuration.ini ${CMAKE_CURRENT_BINARY_DIR}/configuration.ini COPYONLY)

# Create an executable
add_executable(ev_hogging ${SOURCE_FILES})

# Link against libraries
target_link_libraries(ev_hogging spdlog boost_system boost_filesystem ${ODBC_LIBRARIES})