cmake_minimum_required(VERSION 3.3)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_library(ZMQ_LIB zmq)
if(NOT ZMQ_LIB)
    message(FATAL_ERROR "zmq library not found")
endif()

find_library(GFLAGS_LIB gflags)
if(NOT GFLAGS_LIB)
    message(FATAL_ERROR "gflags library not found")
endif()

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

include_directories(../common/communication)

set(SOURCE_FILES main.cpp ../common/communication/zhelpers.hpp ../common/communication/zmq.hpp Client.cpp Client.h)
add_executable(client ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(client ${ZMQ_LIB} gflags communication rpcMessages ${PROTOBUF_LIBRARY})
