cmake_minimum_required(VERSION 3.10)
project(FinanceApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    main.cpp
    facades.cpp
    analytics.cpp
    export_visitors.cpp
)

add_executable(finance_app ${SOURCES})