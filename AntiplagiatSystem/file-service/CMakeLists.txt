cmake_minimum_required(VERSION 3.10)
project(file_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-deprecated-declarations")

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
    src/file_handler.cpp
    src/database.cpp
)

add_executable(file_service ${SOURCES})

target_include_directories(file_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PQXX_INCLUDE_DIR}
    ${OpenSSL_INCLUDE_DIR}
)

target_link_libraries(file_service
    ${PQXX_LIBRARY}
    ${PQ_LIBRARY}
    cpprestsdk::cpprest
    OpenSSL::Crypto
    OpenSSL::SSL
    pthread
)

install(TARGETS file_service DESTINATION /app)