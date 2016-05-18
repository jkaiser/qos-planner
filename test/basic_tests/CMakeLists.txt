include_directories(../../src/common)

file(GLOB SRCS *.cpp)

add_executable(runBasicTests ${SRCS})

target_link_libraries(runBasicTests ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} libgtest libgmock common)
install(TARGETS runBasicTests DESTINATION bin)
