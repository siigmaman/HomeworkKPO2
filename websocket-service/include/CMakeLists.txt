cmake_minimum_required(VERSION 3.16)
project(websocket-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
)

add_executable(websocket-service
    src/main.cpp
    src/websocket_server.cpp
    src/notification_manager.cpp
)

target_link_libraries(websocket-service
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)