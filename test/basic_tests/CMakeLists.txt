include_directories(../../src/common)

file(GLOB SRCS *.cpp)

add_executable(runBasicTests ${SRCS})
target_link_libraries(runBasicTests gtest gmock common)

# small helper to run the test within the CLion IDE...
add_executable(runBasicTests_direct ${SRCS})
target_link_libraries(runBasicTests_direct gtest_main gmock common)
