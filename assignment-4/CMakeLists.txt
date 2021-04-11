cmake_minimum_required(VERSION 3.13)

project(assignment4 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(sqlite3)
find_package(CryptoPP)

add_library(sqlite_orm INTERFACE)
target_sources(sqlite_orm INTERFACE sqlite_orm/sqlite_orm.h)

add_library(httplib INTERFACE)
target_sources(httplib INTERFACE httplib/httplib.h)

add_executable(assignment4 
    main.cpp
    car_type.h
    car.h
    driver.h
    order.h
    passenger.h
)

target_link_libraries(assignment4 sqlite_orm httplib Threads::Threads sqlite3 cryptopp)