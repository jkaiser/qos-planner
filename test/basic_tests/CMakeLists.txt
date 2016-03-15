include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../../src/common)

add_executable(runBasicTests basics.cpp cluster_state.cpp)

target_link_libraries(runBasicTests gtest gtest_main common)
