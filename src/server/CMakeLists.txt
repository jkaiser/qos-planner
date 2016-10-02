cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})


find_library(ZMQ_LIB zmq)
if(NOT ZMQ_LIB)
    message(FATAL_ERROR "zmq library not found")
endif()

find_library(GFLAGS_LIB gflags)
if(NOT GFLAGS_LIB)
    message(FATAL_ERROR "gflags library not found")
endif()


find_library(G3LOG_LIB g3logger)
if(NOT G3LOG_LIB)
    message(FATAL_ERROR "g3log library not found")
endif()

include_directories(../common)
include_directories(../common/communication)

set(SOURCE_FILES main.cpp Server.cpp Server.h)
add_executable(server ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(server ${ZMQ_LIB} ${GFLAGS_LIB} ${G3LOG_LIB} ${PROTOBUF_LIBRARY} common communication rpcMessages  ${CMAKE_THREAD_LIBS_INIT})
