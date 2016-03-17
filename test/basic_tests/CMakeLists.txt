include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../../src/common)

add_executable(runBasicTests basics.cpp cluster_state.cpp MemoryScheduleStateTest.cpp JobPriorityQueueTest.cpp JobMonitorTest.cpp)

target_link_libraries(runBasicTests gtest gmock gtest_main common)
