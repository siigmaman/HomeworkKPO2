cmake_minimum_required(VERSION 3.10)
project(api_gateway)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

set(SOURCES
    src/main.cpp
    src/gateway.cpp
)

add_executable(api_gateway ${SOURCES})

target_include_directories(api_gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenSSL_INCLUDE_DIR}
)

target_link_libraries(api_gateway
    cpprestsdk::cpprest
    OpenSSL::Crypto
    OpenSSL::SSL
    Threads::Threads
    pthread
)

install(TARGETS api_gateway DESTINATION /app)