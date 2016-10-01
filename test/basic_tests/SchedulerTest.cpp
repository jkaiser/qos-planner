//
// Created by jkaiser on 22.03.16.
//


#include "gtest/gtest.h"
#include "MockScheduleState.h"
#include "MockClusterState.h"
#include "MockJobMonitor.h"
#include "MockLustre.h"

#include <JobSchedulerDynWorkloads.h>

using ::testing::_;

class SchedulerTest : public ::testing::Test {

protected:
    common::MockClusterState *mock_cluster_state;
    common::MockScheduleState *mock_scheduler_state;
    common::MockJobMonitor *mock_job_monitor;
    common::MockLustre *mock_lustre;

    std::shared_ptr<common::ClusterState> mocked_cstate;
    std::shared_ptr<common::ScheduleState> mocked_sstate;
    std::shared_ptr<common::MockLustre> mocked_ll;
    std::shared_ptr<common::JobMonitor> mocked_jobmon;

    virtual void SetUp() {
        mock_cluster_state = new common::MockClusterState();
        mock_scheduler_state = new common::MockScheduleState();
        mock_job_monitor = new common::MockJobMonitor();
        mock_lustre = new common::MockLustre();

        mocked_cstate.reset(mock_cluster_state);
        mocked_sstate.reset(mock_scheduler_state);
        mocked_ll.reset(mock_lustre);
        mocked_jobmon.reset(mock_job_monitor);
    }

    virtual void TearDown() {
        mocked_cstate.reset();
        mocked_sstate.reset();
        mocked_ll.reset();
        mocked_jobmon.reset();

        mock_cluster_state = nullptr;
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





TEST_F(SchedulerTest, InitShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);
    EXPECT_TRUE(scheduler.Init());
}

TEST_F(SchedulerTest, TeardownShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);
    scheduler.Init();
    EXPECT_TRUE(scheduler.TearDown());
}

TEST_F(SchedulerTest, ScheduleOnNonExistingClusterShouldFail) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).Times(0);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_FALSE(scheduler.ScheduleJob(*job));
    delete job;
}

TEST_F(SchedulerTest, ScheduleJobWithEnoughResourcesShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    scheduler.Init();
    EXPECT_TRUE(scheduler.ScheduleJob(*job)) << "job should be scheduled";
    delete job;
}

TEST_F(SchedulerTest, ScheduleOnEdgeJobShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job = CreateJob("job", 1, 100);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    scheduler.Init();
    EXPECT_TRUE(scheduler.ScheduleJob(*job)) << "job should be scheduled";
    delete job;
}

TEST_F(SchedulerTest, ScheduleTooBigJob) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> touched_osts_for_job = CreateSingleOstList();
    job->setOsts(touched_osts_for_job);

    std::vector<std::string> osts_in_system = touched_osts_for_job;
    common::OSTWorkload ost_workload = {touched_osts_for_job[0], 0, 100};

    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state, getOstState(touched_osts_for_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).Times(0);

    scheduler.Init();
    EXPECT_FALSE(scheduler.ScheduleJob(*job)) << "job should not be scheduled";
    delete job;
}

TEST_F(SchedulerTest, RemoveExistingJobShouldSucceed) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::vector<std::string> osts_in_system = osts_touched_by_job;
    common::OSTWorkload ost_workload = {osts_touched_by_job[0], 0, 100};

    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&osts_in_system));
    ON_CALL(*mock_cluster_state, getOstState(osts_touched_by_job[0], _)).WillByDefault(
            testing::DoAll(testing::SetArgPointee<1>(ost_workload), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_scheduler_state, RemoveJob(job->getJobid())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_scheduler_state, GetJobStatus(job->getJobid(), _)).WillOnce(testing::DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED),testing::Return(true)));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_CALL(*mock_job_monitor, UnregisterJob(_)).Times(1);

    scheduler.Init();
    scheduler.ScheduleJob(*job);
    EXPECT_TRUE(scheduler.RemoveJob(job->getJobid())) << "Call should succeed if job exist";
    delete job;
}

TEST_F(SchedulerTest, RemoveNonExistingJobShouldFail) {
    common::JobSchedulerDynWorkloads scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_CALL(*mock_job_monitor, UnregisterJob(_)).Times(0);

    scheduler.Init();
    EXPECT_FALSE(scheduler.RemoveJob(job->getJobid())) << "Call should fail if job doesn't exist";
    delete job;
}
