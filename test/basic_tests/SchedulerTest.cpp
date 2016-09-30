//
// Created by jkaiser on 22.03.16.
//


#include "gtest/gtest.h"
#include "MockScheduleState.h"
#include "MockClusterState.h"
#include "MockJobMonitor.h"
#include "MockLustre.h"

#include <Scheduler.h>


using ::testing::_;


class SchedulerTest : public ::testing::Test {

protected:
    common::MockClusterState *mock_cluster_state = new common::MockClusterState();
    common::MockScheduleState *mock_scheduler_state = new common::MockScheduleState();
    common::MockJobMonitor *mock_job_monitor = new common::MockJobMonitor();
    common::MockLustre *mock_lustre = new common::MockLustre();

    std::shared_ptr<common::ClusterState> mocked_cstate;
    std::shared_ptr<common::ScheduleState> mocked_sstate;
    std::shared_ptr<common::MockLustre> mocked_ll;
    std::shared_ptr<common::JobMonitor> mocked_jobmon;

    virtual void SetUp() {
        mock_cluster_state = new common::MockClusterState();
        mock_scheduler_state = new common::MockScheduleState();
        mock_job_monitor = new common::MockJobMonitor();
        mock_lustre = new common::MockLustre();

        mocked_cstate = std::shared_ptr<common::ClusterState>(mock_cluster_state);
        mocked_sstate = std::shared_ptr<common::ScheduleState>(mock_scheduler_state);
        mocked_ll = std::shared_ptr<common::MockLustre>(mock_lustre);
        mocked_jobmon = std::shared_ptr<common::JobMonitor>(mock_job_monitor);
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
};

TEST_F(SchedulerTest, ScheduleOnEmptyCluster) {
    common::Scheduler scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state, AddJob(_,_,_)).Times(0);

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);
    std::vector<std::string> osts = {"OST_a"};
    job1->setOsts(osts);

    EXPECT_FALSE(scheduler.ScheduleJob(*job1));
    delete job1;
}

std::vector<std::string> CreateSingleOstList() {
    return {"OST"};
}

TEST_F(SchedulerTest, ScheduleSimpleJob) {
    common::Scheduler scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);
    std::vector<std::string> osts = CreateSingleOstList();
    job1->setOsts(osts);

    common::OSTWorkload node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};


    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&nodes));
    ON_CALL(*mock_cluster_state, getOstState(osts[0], _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(node_state), testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state, AddJob(_,_,_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_TRUE(scheduler.Init());
    EXPECT_TRUE(scheduler.ScheduleJob(*job1)) << "job should be scheduled";
    delete job1;
}


TEST_F(SchedulerTest, ScheduleTooBigJob) {
    common::Scheduler scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);
    std::vector<std::string> osts = CreateSingleOstList();
    job1->setOsts(osts);

    common::OSTWorkload node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};


    ON_CALL(*mock_cluster_state, GetOSTList()).WillByDefault(testing::Return(&nodes));
    ON_CALL(*mock_cluster_state, getOstState(osts[0], _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(node_state), testing::Return(true)));

    EXPECT_CALL(*mock_job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(*mock_scheduler_state, AddJob(_,_,_)).Times(0);

    EXPECT_FALSE(scheduler.ScheduleJob(*job1)) << "job should not be scheduled";
    delete job1;
}


TEST_F(SchedulerTest, RemoveNonExistingJob) {
    common::Scheduler scheduler(mocked_sstate, mocked_jobmon, mocked_cstate, mocked_ll);

    common::OSTWorkload node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);
    std::vector<std::string> osts = CreateSingleOstList();
    job1->setOsts(osts);

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(*mock_job_monitor, UnregisterJob(_)).Times(0);

    EXPECT_FALSE(scheduler.RemoveJob(job1->getJobid())) << "Call should fail if job doesn't exist";
    delete job1;
}
