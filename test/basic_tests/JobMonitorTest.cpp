//
// Created by jkaiser on 18.03.16.
//


#include "gmock/gmock.h"

#include <Lustre.h>
#include <JobMonitor.h>

using ::testing::_;

namespace common {

class MockScheduleState : public MemoryScheduleState {
public:
    MOCK_METHOD2(GetJobEnd, bool(std::string, std::chrono::system_clock::time_point*));
    MOCK_METHOD2(GetJobStatus, bool(std::string, Job::JobState*));
    MOCK_METHOD2(UpdateJob, bool(std::string, Job::JobState));
    MOCK_METHOD2(GetJobThroughput, bool(std::string, uint32_t*));
    MOCK_METHOD0(GetAllJobs, std::map<std::string, Job*> *());
};

class MockLustre : public LocalLustre {
public:
    MOCK_METHOD3(StartJobTbfRule, bool(std::string, std::string, uint32_t));
    MOCK_METHOD2(StopJobTbfRule, bool(std::string, std::string));
};
}


TEST(JobMonitor, InitGetJobs) {
    common::MockScheduleState scheduleState;
    common::LocalLustre lustre;
    common::JobMonitor jobMonitor(&scheduleState, &lustre, 3);

    std::map<std::string, common::Job*> *jobmap = new std::map<std::string, common::Job*>();
    (*jobmap)["job1"] = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                42);
    (*jobmap)["job1"]->setState(common::Job::SCHEDULED);

    EXPECT_CALL(scheduleState, GetAllJobs()).WillOnce(testing::Return(jobmap));

    EXPECT_TRUE(jobMonitor.Init());
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST(JobMonitor, InitTeardown) {
    common::MemoryScheduleState scheduleState;
    common::LocalLustre lustre;
    common::JobMonitor jobMonitor(&scheduleState, &lustre, 3);

    EXPECT_TRUE(jobMonitor.Init());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST(JobMonitor, StartStopJob) {
    // The Start & Stop call are private, so I trigger them by small start-stop times
    common::MockScheduleState scheduleState;
    common::MockLustre lustre;
    lustre.Init();
    common::JobMonitor jobMonitor(&scheduleState, &lustre, 1);

    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                42);

    EXPECT_CALL(scheduleState, GetJobStatus(job1->getJobid(), _))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), testing::Return(true)))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::ACTIVE), testing::Return(true)));
    EXPECT_CALL(scheduleState, UpdateJob(job1->getJobid(), _)).Times(2).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(scheduleState, GetJobEnd(job1->getJobid(), _)).WillOnce(testing::DoAll(testing::SetArgPointee<1>(job1->getTend()), testing::Return(true)));
    ON_CALL(scheduleState, GetJobThroughput(job1->getJobid(), _)).WillByDefault(testing::Return(true));
    EXPECT_CALL(lustre, StartJobTbfRule(job1->getJobid(), _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(lustre, StopJobTbfRule(job1->getJobid(), _)).WillOnce(testing::Return(true));

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST(JobMonitor, RegisterJob) {
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
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), testing::Return(true)));
    EXPECT_CALL(scheduleState, UpdateJob(job1->getJobid(), _)).Times(1).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(scheduleState, GetJobThroughput(job1->getJobid(), _)).WillOnce(testing::Return(true));
    EXPECT_CALL(lustre, StartJobTbfRule(job1->getJobid(), _, _)).WillOnce(testing::Return(true));

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
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
