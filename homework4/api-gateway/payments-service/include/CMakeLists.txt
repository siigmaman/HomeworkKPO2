cmake_minimum_required(VERSION 3.16)
project(payments-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pqxx REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include
)

add_executable(payments-service
    src/main.cpp
    src/payment_service.cpp
    src/database.cpp
    src/message_queue.cpp
    src/inbox_processor.cpp
    src/outbox_processor.cpp
)

target_link_libraries(payments-service
    pqxx::pqxx
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)