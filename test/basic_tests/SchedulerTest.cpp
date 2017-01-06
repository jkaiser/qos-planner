//
// Created by jkaiser on 22.03.16.
//


#include "gtest/gtest.h"
#include "mock_classes/MockScheduleState.h"
#include "mock_classes/MockClusterState.h"
#include "mock_classes/MockJobMonitor.h"
#include "mock_classes/MockLustre.h"

#include <JobSchedulerDynWorkloads.h>

using ::testing::_;

class SchedulerTest : public ::testing::Test {

protected:
    common::MockClusterState *mock_cluster_state_;
    common::MockScheduleState *mock_scheduler_state_;
    common::MockJobMonitor *mock_job_monitor_;
    common::MockLustre *mock_lustre_;

    std::shared_ptr<common::ClusterState> mocked_cstate_;
    std::shared_ptr<common::ScheduleState> mocked_sstate_;
    std::shared_ptr<common::MockLustre> mocked_ll_;
    std::shared_ptr<common::JobMonitor> mocked_jobmon_;

    virtual void SetUp() {
        mock_cluster_state_ = new common::MockClusterState();
        mock_scheduler_state_ = new common::MockScheduleState();
        mock_job_monitor_ = new common::MockJobMonitor();
        mock_lustre_ = new common::MockLustre();

        mocked_cstate_.reset(mock_cluster_state_);
        mocked_sstate_.reset(mock_scheduler_state_);
        mocked_ll_.reset(mock_lustre_);
        mocked_jobmon_.reset(mock_job_monitor_);
    }

    virtual void TearDown() {
        mocked_cstate_.reset();
        mocked_sstate_.reset();
        mocked_ll_.reset();
        mocked_jobmon_.reset();

        mock_cluster_state_ = nullptr;
        mock_scheduler_state_ = nullptr;
        mock_job_monitor_ = nullptr;
        mock_lustre_ = nullptr;
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





TEST_F(SchedulerTest, InitShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);
    EXPECT_TRUE(scheduler.Init());
}

TEST_F(SchedulerTest, TeardownShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);
    scheduler.Init();
    EXPECT_TRUE(scheduler.Teardown());
}

TEST_F(SchedulerTest, ScheduleOnNonExistingClusterShouldFail) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).Times(0);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_FALSE(scheduler.ScheduleJob(*job));
    delete job;
}

TEST_F(SchedulerTest, ScheduleJobWithEnoughResourcesShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state_, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state_, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).WillOnce(testing::Return(true));

    scheduler.Init();
    EXPECT_TRUE(scheduler.ScheduleJob(*job)) << "job should be scheduled";
    delete job;
}

TEST_F(SchedulerTest, ScheduleOnEdgeJobShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    auto job = CreateJob("job", 1, 100);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state_, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state_, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).WillOnce(testing::Return(true));

    scheduler.Init();
    EXPECT_TRUE(scheduler.ScheduleJob(*job)) << "job should be scheduled";
    delete job;
}

TEST_F(SchedulerTest, ScheduleTooBigJob) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> touched_osts_for_job = CreateSingleOstList();
    job->setOsts(touched_osts_for_job);

    std::vector<std::string> osts_in_system = touched_osts_for_job;
    common::OSTWorkload ost_workload = {touched_osts_for_job[0], 0, 100};

    ON_CALL(*mock_cluster_state_, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state_, getOstState(touched_osts_for_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).Times(0);

    scheduler.Init();
    EXPECT_FALSE(scheduler.ScheduleJob(*job)) << "job should not be scheduled";
    delete job;
}

TEST_F(SchedulerTest, RemoveExistingJobShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state_, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state_, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_scheduler_state_, RemoveJob(job->getJobid())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_scheduler_state_, GetJobStatus(job->getJobid(), _)).WillOnce(testing::DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED),testing::Return(true)));
    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_CALL(*mock_job_monitor_, UnregisterJob(_)).Times(1);

    scheduler.Init();
    scheduler.ScheduleJob(*job);
    EXPECT_TRUE(scheduler.RemoveJob(job->getJobid())) << "Call should succeed if job exist";
    delete job;
}

TEST_F(SchedulerTest, RemoveNonExistingJobShouldFail) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate_, mocked_jobmon_, mocked_cstate_, mocked_ll_);

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_CALL(*mock_job_monitor_, UnregisterJob(_)).Times(0);

    scheduler.Init();
    EXPECT_FALSE(scheduler.RemoveJob(job->getJobid())) << "Call should fail if job doesn't exist";
    delete job;
}
