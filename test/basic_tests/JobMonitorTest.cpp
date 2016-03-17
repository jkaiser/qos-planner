//
// Created by jkaiser on 18.03.16.
//


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <JobMonitor.h>


namespace common {

class MockScheduleState : public MemoryScheduleState {
public:
    MOCK_METHOD2(UpdateJob, bool(std::string, Job::JobState));
};
}

TEST(JobMonitor, InitTeardown) {
    common::MemoryScheduleState scheduleState;

    common::JobMonitor jobMonitor(&scheduleState);

    EXPECT_TRUE(jobMonitor.Init());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST(JobMonitor, RegisterUnregisterJob) {
    common::MockScheduleState scheduleState;
    common::JobMonitor jobMonitor(&scheduleState);

    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = new common::Job("job1",
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now() + std::chrono::hours(1),
            42);


    EXPECT_CALL(scheduleState, UpdateJob(job1->getJobid(), common::Job::ACTIVE)).Times(1);

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_TRUE(jobMonitor.TearDown());
}

//
//int main(int argc, char** argv) {
//    ::testing::InitGoogleMock(&argc, argv);
//    return RUN_ALL_TESTS();

