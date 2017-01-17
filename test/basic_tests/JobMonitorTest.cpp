/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */


#include "gmock/gmock.h"

#include "mock_classes/MockScheduleState.h"
#include "mock_classes/MockRuleManager.h"
#include <JobMonitor.h>

#include <spdlog/spdlog.h>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;

class JobMonitorTest : public ::testing::Test {

protected:

    common::MockRuleManager *mock_rule_manager_;
    common::MockScheduleState *schedule_state_ = new common::MockScheduleState();

    std::shared_ptr<common::MockRuleManager> mocked_rule_manager_;
    std::shared_ptr<common::ScheduleState> mocked_schedule_state_;

    virtual void SetUp() {
        if (!spdlog::get("console")) {
            auto console = spdlog::stdout_logger_mt("console");
            spdlog::set_level(spdlog::level::critical);
        }

        mock_rule_manager_ = new common::MockRuleManager();
        schedule_state_ = new common::MockScheduleState();
        mocked_rule_manager_.reset(mock_rule_manager_);
        mocked_schedule_state_.reset(schedule_state_);
    }

    common::Job *createSimpleJob() {
        auto job = new common::Job("job1",
                        std::chrono::system_clock::now(),
                        std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                        42);
        std::vector<std::string> ost_ids = {"ost_id"};
        job->setOsts(ost_ids);
        return job;
    }
};

TEST_F(JobMonitorTest, InitGetJobs) {

    common::JobMonitor jobMonitor(mocked_schedule_state_, mocked_rule_manager_);

    std::map<std::string, common::Job*> *jobmap = new std::map<std::string, common::Job*>();
    (*jobmap)["job1"] = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                                42);
    (*jobmap)["job1"]->setState(common::Job::SCHEDULED);

    EXPECT_CALL(*schedule_state_, GetAllJobs()).WillOnce(Return(jobmap));

    EXPECT_TRUE(jobMonitor.Init());
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST_F(JobMonitorTest, InitTeardown) {

    auto job_map = new std::map<std::string, common::Job *>();
    ON_CALL(*schedule_state_, GetAllJobs()).WillByDefault(Return(job_map));

    common::JobMonitor jobMonitor(mocked_schedule_state_, mocked_rule_manager_);

    EXPECT_TRUE(jobMonitor.Init());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(jobMonitor.TearDown());
}

TEST_F(JobMonitorTest, StartStopJob) {
    // The Start & Stop call are private, so I trigger them by small start-stop times

    common::JobMonitor jobMonitor(mocked_schedule_state_, mocked_rule_manager_, 1);

    auto job_map = new std::map<std::string, common::Job *>();
    ON_CALL(*schedule_state_, GetAllJobs()).WillByDefault(testing::Return(job_map));

    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = createSimpleJob();

    EXPECT_CALL(*schedule_state_, GetJobStatus(job1->getJobid(), _))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), Return(true)))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::ACTIVE), Return(true)));
    EXPECT_CALL(*schedule_state_, UpdateJob(job1->getJobid(), _)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*schedule_state_, GetJobEnd(job1->getJobid(), _)).WillOnce(testing::DoAll(testing::SetArgPointee<1>(
            job1->GetEndTime()), testing::Return(true)));
    ON_CALL(*schedule_state_, GetJobThroughput(job1->getJobid(), _)).WillByDefault(testing::Return(true));
    ON_CALL(*schedule_state_, GetJobOstIds(job1->getJobid(), _)).WillByDefault(DoAll(testing::SetArgReferee<1>(job1->getOsts()), Return(true)));
    EXPECT_CALL(*mocked_rule_manager_, SetRules(_, job1->getJobid(), _)).WillOnce(Return(true));
    EXPECT_CALL(*mocked_rule_manager_, RemoveRules(job1->getJobid())).WillOnce(Return(true));

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(jobMonitor.TearDown());

    delete job1;
}

TEST_F(JobMonitorTest, RegisterJob) {

    common::JobMonitor jobMonitor(mocked_schedule_state_, mocked_rule_manager_, 1);

    auto job_map = new std::map<std::string, common::Job *>();
    ON_CALL(*schedule_state_, GetAllJobs()).WillByDefault(testing::Return(job_map));
    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = createSimpleJob();

    EXPECT_CALL(*schedule_state_, GetJobStatus(job1->getJobid(), _))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), Return(true)));
    EXPECT_CALL(*schedule_state_, UpdateJob(job1->getJobid(), _)).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*schedule_state_, GetJobThroughput(job1->getJobid(), _)).WillOnce(Return(true));
    EXPECT_CALL(*mocked_rule_manager_, SetRules(_, job1->getJobid(), _)).WillOnce(Return(true));


    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(jobMonitor.TearDown());

    delete job1;
}

TEST_F(JobMonitorTest, RegisterUnregisterJob) {

    common::JobMonitor jobMonitor(mocked_schedule_state_, mocked_rule_manager_, 1);

    auto job_map = new std::map<std::string, common::Job *>();
    ON_CALL(*schedule_state_, GetAllJobs()).WillByDefault(testing::Return(job_map));

    EXPECT_TRUE(jobMonitor.Init());

    auto job1 = new common::Job("job1",
                                std::chrono::system_clock::now(),
                                std::chrono::system_clock::now() + std::chrono::hours(1),   // long duration to prevent internal cleanup
                                42);

    ON_CALL(*schedule_state_, GetJobEnd(_, _)).WillByDefault(DoAll(testing::SetArgPointee<1>(job1->GetEndTime()),Return(true)));

    EXPECT_CALL(*schedule_state_, GetJobStatus(job1->getJobid(), _))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED), Return(true)))
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::ACTIVE), Return(true))) // unregister
            .WillOnce(DoAll(testing::SetArgPointee<1>(common::Job::ACTIVE), Return(true))); // call StopJob in unregister
    EXPECT_CALL(*schedule_state_, UpdateJob(job1->getJobid(), _)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*schedule_state_, GetJobThroughput(job1->getJobid(), _)).WillOnce(Return(true));
    ON_CALL(*schedule_state_, GetJobOstIds(job1->getJobid(), _)).WillByDefault(DoAll(testing::SetArgReferee<1>(job1->getOsts()), Return(true)));
    EXPECT_CALL(*mocked_rule_manager_, SetRules(_, job1->getJobid(), _)).WillOnce(Return(true));
    EXPECT_CALL(*mocked_rule_manager_, RemoveRules(job1->getJobid())).WillOnce(Return(true));

    EXPECT_TRUE(jobMonitor.RegisterJob(*job1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_TRUE(jobMonitor.UnregisterJob(*job1));
    EXPECT_TRUE(jobMonitor.TearDown());
}
