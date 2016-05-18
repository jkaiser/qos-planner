#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../../src/common)
include_directories(../../src/common)

add_executable(runBasicTests cluster_state.cpp MemoryScheduleStateTest.cpp JobPriorityQueueTest.cpp JobMonitorTest.cpp SchedulerTest.cpp MockScheduleState.h MockClusterState.h MockClusterState.cpp MockClusterState.h MockScheduleState.cpp MockScheduleState.h MockClusterState.cpp MockClusterState.h MockJobMonitor.cpp MockJobMonitor.h MockLustre.cpp MockLustre.h)

#target_link_libraries(runBasicTests gtest gmock gtest_main common) # outcommented to test automatic download
add_dependencies(runBasicTests gtest gmock)
target_link_libraries(runBasicTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} common)
