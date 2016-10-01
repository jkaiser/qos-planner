cmake_minimum_required(VERSION 3.3)

add_subdirectory(rpc)
add_subdirectory(communication)

set(SOURCE_FILES ClusterState.h ClusterState.cpp Job.h Job.cpp ScheduleState.cpp ScheduleState.h JobMonitor.cpp JobMonitor.h JobPriorityQueue.cpp JobPriorityQueue.h Lustre.cpp Lustre.h JobSchedulerDynWorkloads.cpp JobSchedulerDynWorkloads.h Planner.cpp Planner.h Job.cpp util.h JobScheduler.cpp JobScheduler.h JobSchedulerStaticWorkloads.cpp JobSchedulerStaticWorkloads.h)
add_library(common ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(common ${PROTOBUF_LIBRARY} communication rpcMessages)
