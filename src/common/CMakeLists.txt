cmake_minimum_required(VERSION 3.3)

find_library(json REQUIRED)

add_subdirectory(rpc)
add_subdirectory(communication)

set(SOURCE_FILES ClusterState.h ClusterState.cpp Job.h Job.cpp ScheduleState.cpp ScheduleState.h JobMonitor.cpp JobMonitor.h JobPriorityQueue.cpp JobPriorityQueue.h Lustre.cpp Lustre.h JobSchedulerDynWorkloads.cpp JobSchedulerDynWorkloads.h Planner.cpp Planner.h Job.cpp util.h JobScheduler.cpp JobScheduler.h JobSchedulerStaticWorkloads.cpp JobSchedulerStaticWorkloads.h OSTLimitConfigParser.cpp OSTLimitConfigParser.h ListJobsFormatter.cpp ListJobsFormatter.h RuleSetter.h SSHRuleSetter.cpp SSHRuleSetter.h RuleManager.cpp RuleManager.h OstIpsCache.cpp OstIpsCache.h OstIdsConverter.cpp OstIdsConverter.h MemoryScheduleState.h MemoryScheduleState.cpp)
add_library(common ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(common ${PROTOBUF_LIBRARY} ${JSON_LIBRARY} communication rpcMessages)
