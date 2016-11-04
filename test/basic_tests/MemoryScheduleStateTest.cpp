//
// Created by jkaiser on 16.03.16.
//

#include "gtest/gtest.h"

#include <ScheduleState.h>

#include <spdlog/spdlog.h>


class MemScheduleStateTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        if (!spdlog::get("console")) {
            auto console = spdlog::stdout_logger_mt("console");
            spdlog::set_level(spdlog::level::critical);
        }

    }
};

TEST_F(MemScheduleStateTest, Init) {

    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());
    EXPECT_TRUE(mss.TearDown());
}

TEST_F(MemScheduleStateTest, AddJob) {
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

TEST_F(MemScheduleStateTest, RemoveJob) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a", "OST_b", "OST_c"};
    EXPECT_TRUE(mss.AddJob(j->getJobid(), *j, osts));

    EXPECT_TRUE(mss.RemoveJob(j->getJobid())) << "Remove of existing job must succeed";
    EXPECT_FALSE(mss.RemoveJob(j->getJobid()))<< "Remove of nonexisting job must fail";
}

TEST_F(MemScheduleStateTest, AddJobTwice) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a", "OST_b", "OST_C"};
    EXPECT_TRUE(mss.AddJob("foo", *j, osts));
    EXPECT_FALSE(mss.AddJob("foo", *j, osts)) << "No two jobs for the same ID";

    EXPECT_TRUE(mss.TearDown());
};

TEST_F(MemScheduleStateTest, Reset) {
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


TEST_F(MemScheduleStateTest, UpdateJob) {
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

TEST_F(MemScheduleStateTest, GetJobOsts) {
    common::MemoryScheduleState mss;
    EXPECT_TRUE(mss.Init());

    common::Job *j = new common::Job("foo", std::chrono::system_clock::now(), std::chrono::system_clock::now(), 42);
    std::vector<std::string> osts = {"OST_a", "OST_b", "OST_C"};
    j->setOsts(osts);
    mss.AddJob("foo", *j, osts);

    std::vector<std::string> returned_osts;
    mss.GetJobOstIds("foo", returned_osts);

    std::set<std::string> ost_set;
    for (auto &ost : osts) {
        ost_set.insert(ost);
    }

    for (auto &returned_ost : returned_osts) {
        auto it = ost_set.find(returned_ost);
        ASSERT_TRUE(it != ost_set.end()) << "Returned invalid ost id: " << returned_ost;
        ost_set.erase(it);
    }

    ASSERT_EQ(ost_set.size(), 0) << "Not all osts were returned";

    EXPECT_TRUE(mss.TearDown());
};