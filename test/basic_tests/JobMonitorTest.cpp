//
// Created by jkaiser on 18.03.16.
//


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <Lustre.h>
#include <JobMonitor.h>

using ::testing::_;

namespace common {

class MockScheduleState : public MemoryScheduleState {
public:
    MOCK_METHOD2(GetJobStatus, bool(std::string, Job::JobState*));
    MOCK_METHOD2(UpdateJob, bool(std::string, Job::JobState));
    MOCK_METHOD2(GetJobThroughput, bool(std::string, uint32_t*));
};

class MockLustre : public LocalLustre {
public:
    MOCK_METHOD3(StartJobTbfRule, bool(std::string, std::string, uint32_t));
    MOCK_METHOD2(StopJobTbfRule, bool(std::string, std::string));
};
}

TEST(JobMonitor, InitTeardown) {
    common::MemoryScheduleState scheduleState;
    common::LocalLustre lustre;
    common::JobMonitor jobMonitor(&scheduleState, &lustre, 3);

    EXPECT_TRUE(jobMonitor.Init());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(jobMonitor.TearDown());
}



TEST(JobMonitor, RegisterUnregisterJob) {
    common::MockScheduleState scheduleState;
    common::MockLustre lustre;
    lustre.Init();
    common::JobMonitor jobMonitor(&scheduleState, &lustre, 1);

    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::hours(1),
                                42);

    EXPECT_CALL(scheduleState, GetJobStatus(job1->getJobid(), _))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), testing::Return(true)))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::ACTIVE), testing::Return(true)));
    EXPECT_CALL(scheduleState, UpdateJob(job1->getJobid(), _)).Times(2).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(scheduleState, GetJobThroughput(job1->getJobid(), _)).WillOnce(testing::Return(true));
    EXPECT_CALL(lustre, StartJobTbfRule(job1->getJobid(), _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(lustre, StopJobTbfRule(job1->getJobid(), _)).WillOnce(testing::Return(true));

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(jobMonitor.UnregisterJob(*job1));
    EXPECT_TRUE(jobMonitor.TearDown());
}
