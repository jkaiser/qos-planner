cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads)
find_library(ZMQ_LIB zmq)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

include_directories(../common)
include_directories(../common/communication)

set(SOURCE_FILES main.cpp Server.cpp Server.h)
add_executable(server ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(server ${ZMQ_LIB} gflags ${PROTOBUF_LIBRARY} common communication rpcMessages  ${CMAKE_THREAD_LIBS_INIT})
