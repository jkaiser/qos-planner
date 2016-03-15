cmake_minimum_required(VERSION 3.3)

add_subdirectory(rpc)
add_subdirectory(communication)

set(SOURCE_FILES ClusterState.h ClusterState.cpp)
add_library(common ${SOURCE_FILES})
