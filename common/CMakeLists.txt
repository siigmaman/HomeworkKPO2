cmake_minimum_required(VERSION 3.16)

add_library(common_lib
    src/database.cpp
)

target_include_directories(common_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(pqxx REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(common_lib
    pqxx::pqxx
    nlohmann_json::nlohmann_json
)