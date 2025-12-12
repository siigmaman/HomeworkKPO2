cmake_minimum_required(VERSION 3.10)
project(analysis_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

find_package(cpprestsdk REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

find_path(PQXX_INCLUDE_DIR pqxx/pqxx)
find_library(PQXX_LIBRARY pqxx)
find_library(PQ_LIBRARY pq)

if(NOT PQXX_INCLUDE_DIR OR NOT PQXX_LIBRARY OR NOT PQ_LIBRARY)
    message(FATAL_ERROR "PostgreSQL libraries not found. Install: sudo apt-get install libpqxx-dev libpq-dev")
endif()

set(SOURCES
    src/main.cpp
    src/analyzer.cpp
    src/database.cpp
)

add_executable(analysis_service ${SOURCES})

target_include_directories(analysis_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PQXX_INCLUDE_DIR}
    ${OpenSSL_INCLUDE_DIR}
)

target_link_libraries(analysis_service
    ${PQXX_LIBRARY}
    ${PQ_LIBRARY}
    cpprestsdk::cpprest
    OpenSSL::Crypto
    OpenSSL::SSL
    pthread
)

install(TARGETS analysis_service DESTINATION /app)