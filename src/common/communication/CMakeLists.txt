cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_library(ZMQ_LIB zmq)
if(NOT ZMQ_LIB)
    message(FATAL_ERROR "zmq library not found")
endif()

set(SOURCE_FILES zhelpers.hpp zhelpers.cpp zmq.hpp zmq.cpp)
add_library(communication ${SOURCE_FILES})
target_link_libraries(communication ${ZMQ_LIB})
