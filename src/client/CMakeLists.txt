cmake_minimum_required(VERSION 3.3)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_library(ZMQ_LIB zmq)

include_directories(../common/communication)

set(SOURCE_FILES main.cpp ../common/communication/zhelpers.hpp ../common/communication/zmq.hpp Client.cpp Client.h)
add_executable(client ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(client ${ZMQ_LIB} gflags communication rpcMessages ${PROTOBUF_LIBRARY})
