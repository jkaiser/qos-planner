cmake_minimum_required(VERSION 3.3)
project(QoS_Planner)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src/common)
add_subdirectory(src/server)
add_subdirectory(src/client)
add_subdirectory(test)
