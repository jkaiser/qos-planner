cmake_minimum_required(VERSION 3.3)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Threads)

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
include_directories(../common)

set(SOURCE_FILES ../common/communication/zhelpers.hpp ../common/communication/zmq.hpp Client.cpp Client.h ReserveRequestBuilder.cpp ReserveRequestBuilder.h RemoveReservationRequestBuilder.cpp RemoveReservationRequestBuilder.h ListReservationsRequestBuilder.cpp ListReservationsRequestBuilder.h StorageReqParser.cpp StorageReqParser.h)

add_library(client_lib ${SOURCE_FILES})
target_link_libraries(client_lib ${ZMQ_LIB} ${GFLAGS_LIB} communication rpcMessages ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})


add_executable(client main.cpp)
TARGET_LINK_LIBRARIES(client client_lib)
