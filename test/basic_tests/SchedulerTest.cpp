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

TEST(Scheduler, ScheduleOnEmptyCluster) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state);

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(schedule_state, AddJob(_,_,_)).Times(0);

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);

    EXPECT_FALSE(scheduler.ScheduleJob(*job1));
    delete job1;
}

TEST(Scheduler, ScheduleSimpleJob) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state);


    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);

    common::NodeState node_state = {"node1", 0, 100};
    std::vector<std::string> nodes = {"node1"};


    ON_CALL(cluster_state, getNodes()).WillByDefault(testing::Return(&nodes));
    ON_CALL(cluster_state, getState(_)).WillByDefault(testing::Return(&node_state));

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, RegisterJob(_)).Times(1);
    EXPECT_CALL(schedule_state, AddJob(_,_,_)).Times(1);

    EXPECT_TRUE(scheduler.ScheduleJob(*job1)) << "job should be scheduled";
    delete job1;
}


TEST(Scheduler, ScheduleTooBigJob) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state);


    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);

    common::NodeState node_state = {"node1", 0, 100};
    std::vector<std::string> nodes = {"node1"};


    ON_CALL(cluster_state, getNodes()).WillByDefault(testing::Return(&nodes));
    ON_CALL(cluster_state, getState(_)).WillByDefault(testing::Return(&node_state));

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(schedule_state, AddJob(_,_,_)).Times(0);

    EXPECT_FALSE(scheduler.ScheduleJob(*job1)) << "job should not be scheduled";
    delete job1;
}


TEST(Scheduler, RemoveNonExistingJob) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state);

    common::NodeState node_state = {"node1", 0, 100};
    std::vector<std::string> nodes = {"node1"};

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, UnregisterJob(_)).Times(0);

    EXPECT_FALSE(scheduler.RemoveJob(job1->getJobid()));
    delete job1;
}
