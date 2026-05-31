cmake_minimum_required(VERSION 3.16)
project(ElectricalPowerMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts PrintSupport)

qt_add_executable(ElectricalPowerMonitor
    mainwindow.h
    mainwindow.cpp
    main.cpp
)

target_link_libraries(ElectricalPowerMonitor PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::PrintSupport
)
