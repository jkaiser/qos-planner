cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(server ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(server ${ZMQ_LIB})