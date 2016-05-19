cmake_minimum_required(VERSION 3.3)

add_subdirectory(rpc)
add_subdirectory(communication)

set(SOURCE_FILES ClusterState.h ClusterState.cpp Job.h ScheduleState.cpp ScheduleState.h JobMonitor.cpp JobMonitor.h JobPriorityQueue.cpp JobPriorityQueue.h Lustre.cpp Lustre.h Scheduler.cpp Scheduler.h)
add_library(common ${SOURCE_FILES})
