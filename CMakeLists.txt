cmake_minimum_required(VERSION 2.8.4)
project(tempserv)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /usr/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    CelSQL.cpp
    CelSQL.h
    main.cpp
    optional.hpp
    Types.h
    Sensor.cpp
    Sensor.h)

include_directories(/usr/local/include/hidapi)
add_executable(tempserv ${SOURCE_FILES})

find_library(HIDAPI_LIB hidapi)
find_library(SQLITE3_LIB sqlite3)

target_link_libraries (tempserv ${HIDAPI_LIB})
target_link_libraries (tempserv ${SQLITE3_LIB})
