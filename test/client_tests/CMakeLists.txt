include_directories(../../src/client)
include_directories(../../src/common)

file(GLOB SRCS *.cpp)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

add_executable(runClientTests ${SRCS} ReserveRequestBuilderTest.cpp RemoveReservationRequestBuilderTest.cpp ListReservationsRequestBuilderTest.cpp)
target_link_libraries(runClientTests gtest gmock rpcMessages ${PROTOBUF_LIBRARY} client_lib)

# small helper to run the test within the CLion IDE...
add_executable(runClientTests_direct ${SRCS})
target_link_libraries(runClientTests_direct gtest_main gmock rpcMessages ${PROTOBUF_LIBRARY} client_lib)
