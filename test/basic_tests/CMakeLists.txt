include_directories(../../src/common)

file(GLOB SRCS *.cpp)

add_executable(runBasicTests ${SRCS} LustreTest.cpp utilTest.cpp JobSchedulerStaticWorkloadsTest.cpp OSTLimitConfigParserTest.cpp ListJobsFormatterTest.cpp RuleManagerTest.cpp mock_classes/MockRuleSetter.cpp mock_classes/MockRuleSetter.h OstIdsConverterTest.cpp OstIpCacheTest.cpp mock_classes/MockOstIpsCache.cpp mock_classes/MockOstIpsCache.h mock_classes/MockOstIdsConverter.cpp mock_classes/MockOstIdsConverter.h mock_classes/MockRuleManager.cpp mock_classes/MockRuleManager.h)
target_link_libraries(runBasicTests gtest gmock common)

# small helper to run the test within the CLion IDE...
add_executable(runBasicTests_direct ${SRCS})
target_link_libraries(runBasicTests_direct gtest_main gmock common)
