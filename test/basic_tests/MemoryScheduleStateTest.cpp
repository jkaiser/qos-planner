//
// Created by jkaiser on 16.03.16.
//

#include "gtest/gtest.h"

#include <ScheduleState.h>



TEST(MemClusterState, Init) {

    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());
    EXPECT_TRUE(mss.TearDown());
}

TEST(MemClusterState, AddJob) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());


    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a", "OST_b", "OST_c"};
    EXPECT_TRUE(mss.AddJob(j->getJobid(), *j, osts));

    std::map<std::string, common::Job*> *job_map = mss.GetAllJobs();
    EXPECT_EQ(1, job_map->size());
    EXPECT_STREQ(j->getJobid().c_str(), job_map->begin()->first.c_str());


    // add second job
    common::Job *j2 = new common::Job("bar", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    osts = {"OST_a"};
    EXPECT_TRUE(mss.AddJob(j2->getJobid(), *j2, osts));

    EXPECT_EQ(2, mss.GetOSTState("OST_a")->size());
    EXPECT_EQ(1, mss.GetOSTState("OST_b")->size());
    EXPECT_EQ(1, mss.GetOSTState("OST_c")->size());

    EXPECT_TRUE(mss.TearDown());
};

TEST(MemClusterState, AddJobTwice) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a", "OST_b", "OST_C"};
    EXPECT_TRUE(mss.AddJob("foo", *j, osts));
    EXPECT_FALSE(mss.AddJob("foo", *j, osts)) << "No two jobs for the same ID";

    EXPECT_TRUE(mss.TearDown());
};

TEST(MemClusterState, Reset) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a"};
    EXPECT_TRUE(mss.AddJob("foo", *j, osts));

    mss.Reset();
    std::map<std::string, common::Job*> *job_map = mss.GetAllJobs();
    EXPECT_EQ(0, job_map->size());
    EXPECT_TRUE(mss.TearDown());
};


TEST(MemClusterState, UpdateJob) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a"};
    EXPECT_TRUE(mss.AddJob("foo", *j, osts));

    mss.UpdateJob("foo", common::Job::DONE);
    std::map<std::string, common::Job*> *job_map = mss.GetAllJobs();
    EXPECT_EQ(common::Job::DONE, job_map->at("foo")->getState());
    EXPECT_TRUE(mss.TearDown());
}
