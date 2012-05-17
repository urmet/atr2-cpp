cmake_minimum_required(VERSION 2.8)

project(atr2-cpp)

include_directories ("${PROJECT_SOURCE_DIR}/include")

add_executable(atr2-cpp src/atr2.cpp src/console-main.cpp)
target_link_libraries(atr2-cpp robot util)

add_subdirectory(src)
