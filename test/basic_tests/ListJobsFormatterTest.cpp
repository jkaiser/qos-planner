//
// Created by Jürgen Kaiser on 04.10.16.
//

#include <gtest/gtest.h>
#include <chrono>

#include <Job.h>
#include <ListJobsFormatter.h>

class ListJobsFormatterTest : public ::testing::Test {

protected:
    common::ListJobsFormatter formatter;
    std::vector<common::Job*> jobs;

    virtual void TearDown() {
        for (int i = 0; i < jobs.size(); ++i) {
            delete jobs[i];
        }
        jobs.clear();
    }

};

TEST_F(ListJobsFormatterTest, FormatEmptyListShouldShowSpecialText) {
    std::shared_ptr<std::string> text = formatter.Format(jobs);

    ASSERT_STREQ("", text->data());
}

TEST_F(ListJobsFormatterTest, FormattedStringMustNotBeEmpty) {

    std::string id = "id";
    std::chrono::time_point<std::chrono::system_clock> tstart = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> tend = tstart;
    int throughput_mbs = 10;

    common::Job *job = new common::Job(id, tstart, tend, throughput_mbs);
    jobs.push_back(job);

    auto text = formatter.Format(jobs);
    ASSERT_TRUE(text->size() > 0);
}