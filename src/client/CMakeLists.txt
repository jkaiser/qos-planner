cmake_minimum_required(VERSION 3.3)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_library(ZMQ_LIB zmq)

set(SOURCE_FILES main.cpp zhelpers.hpp zmq.hpp)

add_executable(client ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(client ${ZMQ_LIB} gflags rpcMessages ${PROTOBUF_LIBRARY})
