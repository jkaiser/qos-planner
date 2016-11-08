//
// Created by Jürgen Kaiser on 01.10.16.
//


#include "gtest/gtest.h"
#include "JobSchedulerStaticWorkloads.h"

#include "mock_classes/MockScheduleState.h"
#include "mock_classes/MockClusterState.h"
#include "mock_classes/MockJobMonitor.h"
#include "mock_classes/MockLustre.h"

using ::testing::_;

class JobSchedStaticWL : public ::testing::Test {

protected:

    std::string ost_limits_cfg_file;
    common::MockScheduleState *mock_scheduler_state;
    common::MockJobMonitor *mock_job_monitor;
    common::MockLustre *mock_lustre;

    std::shared_ptr<common::ScheduleState> mocked_sstate;
    std::shared_ptr<common::MockLustre> mocked_ll;
    std::shared_ptr<common::JobMonitor> mocked_jobmon;

    std::map<std::string, uint32_t> limits;
    std::shared_ptr<common::JobSchedulerStaticWorkloads> scheduler;

    virtual void SetUp() {
        mock_scheduler_state = new common::MockScheduleState();
        mock_job_monitor = new common::MockJobMonitor();
        mock_lustre = new common::MockLustre();

        mocked_sstate.reset(mock_scheduler_state);
        mocked_ll.reset(mock_lustre);
        mocked_jobmon.reset(mock_job_monitor);

        scheduler.reset(new common::JobSchedulerStaticWorkloads(mocked_sstate, mocked_jobmon, mocked_ll, ost_limits_cfg_file));
        scheduler->UpdateLimits(limits);
    }

    virtual void TearDown() {
        scheduler.reset();

        mocked_sstate.reset();
        mocked_ll.reset();
        mocked_jobmon.reset();

        mock_scheduler_state = nullptr;
        mock_job_monitor = nullptr;
        mock_lustre = nullptr;
    }

    std::vector<std::string> CreateSingleOstList() {
        return {"OST_a"};
    }

    common::Job *CreateJob(const std::string &jobname, int duration_in_ms, int min_storage_bandwidth) {
        return new common::Job(jobname,
                               std::chrono::system_clock::now(),
                               std::chrono::system_clock::now() + std::chrono::milliseconds(duration_in_ms),
                               min_storage_bandwidth);
    }
};


TEST_F(JobSchedStaticWL, CanUpdateOSTLimits) {
    scheduler->UpdateLimits(limits);
}

TEST_F(JobSchedStaticWL, ScheduleJobOnEmptyClusterShouldFail) {
    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_FALSE(scheduler->ScheduleJob(*job));
    delete job;
}

TEST_F(JobSchedStaticWL, ScheduleJobOnEmptyClusterWithEnoughResShouldPass) {
    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    auto job2 = CreateJob("job2", 1, 1);
    job2->setOsts(osts_touched_by_job);

    std::map<std::string, uint32_t> ost_resources;
    ost_resources[osts_touched_by_job[0]] = 100;
    scheduler->UpdateLimits(ost_resources);

    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillRepeatedly(testing::Return(true));

    EXPECT_TRUE(scheduler->ScheduleJob(*job));
    EXPECT_TRUE(scheduler->ScheduleJob(*job2));
    delete job;
    delete job2;
}

TEST_F(JobSchedStaticWL, ScheduleJobClusterWithonEdgeResShouldPass) {
    auto job = CreateJob("job", 1, 100);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::map<std::string, uint32_t> ost_resources;
    ost_resources[osts_touched_by_job[0]] = 100;
    scheduler->UpdateLimits(ost_resources);

    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_TRUE(scheduler->ScheduleJob(*job));
    delete job;
}


TEST_F(JobSchedStaticWL, RemoveExistingJobShouldSucceed) {

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::map<std::string, uint32_t> ost_resources;
    ost_resources[osts_touched_by_job[0]] = 100;
    scheduler->UpdateLimits(ost_resources);

    ON_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillByDefault(testing::Return(true));
    ON_CALL(*mock_job_monitor, RegisterJob(_)).WillByDefault(testing::Return(true));
    ON_CALL(*mock_scheduler_state, GetJobStatus(job->getJobid(), _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED),testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state, RemoveJob(job->getJobid())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, UnregisterJob(_)).Times(1);

    scheduler->ScheduleJob(*job);
    EXPECT_TRUE(scheduler->RemoveJob(job->getJobid())) << "Call should succeed if job exist";
    delete job;
}

TEST_F(JobSchedStaticWL, RemoveNonExistingJobShouldFail) {

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_CALL(*mock_job_monitor, UnregisterJob(_)).Times(0);

    EXPECT_FALSE(scheduler->RemoveJob(job->getJobid())) << "Call should fail if job doesn't exist";
    delete job;
}