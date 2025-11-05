cmake_minimum_required(VERSION 3.10)
project(FinanceTests)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(finance_tests test_core.cpp)

target_link_libraries(finance_tests)