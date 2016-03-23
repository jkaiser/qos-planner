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
    common::MockLustre lustre;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state, &lustre);

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, RegisterJob(_)).Times(0);
    EXPECT_CALL(schedule_state, AddJob(_,_,_)).Times(0);

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);
    std::vector<std::string> osts = {"OST_a"};
    job1->setOsts(osts);

    EXPECT_FALSE(scheduler.ScheduleJob(*job1));
    delete job1;
}

TEST(Scheduler, ScheduleSimpleJob) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;
    common::MockLustre lustre;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state, &lustre);


    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1);
    std::vector<std::string> osts = {"OST_a"};
    job1->setOsts(osts);

    common::NodeState node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};


    ON_CALL(cluster_state, getNodes()).WillByDefault(testing::Return(&nodes));
    ON_CALL(cluster_state, getState("OST_a", _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(node_state), testing::Return(true)));

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(schedule_state, AddJob(_,_,_)).WillOnce(testing::Return(true));
    EXPECT_CALL(job_monitor, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_TRUE(scheduler.Init());
    EXPECT_TRUE(scheduler.ScheduleJob(*job1)) << "job should be scheduled";
    delete job1;
}


TEST(Scheduler, ScheduleTooBigJob) {

    common::MockClusterState cluster_state;
    common::MockScheduleState schedule_state;
    common::MockJobMonitor job_monitor;
    common::MockLustre lustre;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state, &lustre);


    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);
    std::vector<std::string> osts = {"OST_a"};
    job1->setOsts(osts);

    common::NodeState node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};


    ON_CALL(cluster_state, getNodes()).WillByDefault(testing::Return(&nodes));
    ON_CALL(cluster_state, getState("OST_a", _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(node_state), testing::Return(true)));

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
    common::MockLustre lustre;

    common::Scheduler scheduler(&schedule_state, &job_monitor, &cluster_state, &lustre);


    common::NodeState node_state = {"OST_a", 0, 100};
    std::vector<std::string> nodes = {"OST_a"};

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                1000);
    std::vector<std::string> osts = {"OST_a"};
    job1->setOsts(osts);

    // we assume an empty cluster -> no job must be scheduled
    EXPECT_CALL(job_monitor, UnregisterJob(_)).Times(0);

    EXPECT_FALSE(scheduler.RemoveJob(job1->getJobid())) << "Call should fail if job doesn't exist";
    delete job1;
}
