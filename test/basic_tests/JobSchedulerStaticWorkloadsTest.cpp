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


#include "gtest/gtest.h"
#include "JobSchedulerStaticWorkloads.h"

#include "mock_classes/MockScheduleState.h"
#include "mock_classes/MockClusterState.h"
#include "mock_classes/MockJobMonitor.h"
#include "mock_classes/MockLustre.h"

using ::testing::_;

class JobSchedStaticWL : public ::testing::Test {

protected:

    std::string ost_limits_cfg_file_;
    common::MockScheduleState *mock_scheduler_state_;
    common::MockJobMonitor *mock_job_monitor_;
    common::MockLustre *mock_lustre_;

    std::shared_ptr<common::ScheduleState> mocked_sstate_;
    std::shared_ptr<common::MockLustre> mocked_ll_;
    std::shared_ptr<common::JobMonitor> mocked_jobmon_;

    std::map<std::string, uint32_t> limits_;
    std::shared_ptr<common::JobSchedulerStaticWorkloads> scheduler_;

    virtual void SetUp() {
        mock_scheduler_state_ = new common::MockScheduleState();
        mock_job_monitor_ = new common::MockJobMonitor();
        mock_lustre_ = new common::MockLustre();

        mocked_sstate_.reset(mock_scheduler_state_);
        mocked_ll_.reset(mock_lustre_);
        mocked_jobmon_.reset(mock_job_monitor_);

        scheduler_.reset(new common::JobSchedulerStaticWorkloads(mocked_sstate_, mocked_jobmon_, mocked_ll_, ost_limits_cfg_file_));
        scheduler_->UpdateLimits(limits_);
    }

    virtual void TearDown() {
        scheduler_.reset();

        mocked_sstate_.reset();
        mocked_ll_.reset();
        mocked_jobmon_.reset();

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


TEST_F(JobSchedStaticWL, CanUpdateOSTLimits) {
    scheduler_->UpdateLimits(limits_);
}

TEST_F(JobSchedStaticWL, ScheduleJobOnEmptyClusterShouldFail) {
    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_FALSE(scheduler_->ScheduleJob(*job));
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
    scheduler_->UpdateLimits(ost_resources);

    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).WillRepeatedly(testing::Return(true));

    EXPECT_TRUE(scheduler_->ScheduleJob(*job));
    EXPECT_TRUE(scheduler_->ScheduleJob(*job2));
    delete job;
    delete job2;
}

TEST_F(JobSchedStaticWL, ScheduleJobClusterWithonEdgeResShouldPass) {
    auto job = CreateJob("job", 1, 100);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::map<std::string, uint32_t> ost_resources;
    ost_resources[osts_touched_by_job[0]] = 100;
    scheduler_->UpdateLimits(ost_resources);

    EXPECT_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor_, RegisterJob(_)).WillOnce(testing::Return(true));

    EXPECT_TRUE(scheduler_->ScheduleJob(*job));
    delete job;
}


TEST_F(JobSchedStaticWL, RemoveExistingJobShouldSucceed) {

    auto job = CreateJob("job", 1, 1);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    std::map<std::string, uint32_t> ost_resources;
    ost_resources[osts_touched_by_job[0]] = 100;
    scheduler_->UpdateLimits(ost_resources);

    ON_CALL(*mock_scheduler_state_, AddJob(_, _, _)).WillByDefault(testing::Return(true));
    ON_CALL(*mock_job_monitor_, RegisterJob(_)).WillByDefault(testing::Return(true));
    ON_CALL(*mock_scheduler_state_, GetJobStatus(job->getJobid(), _)).WillByDefault(testing::DoAll(testing::SetArgPointee<1>(common::Job::SCHEDULED),testing::Return(true)));

    EXPECT_CALL(*mock_scheduler_state_, RemoveJob(job->getJobid())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_job_monitor_, UnregisterJob(_)).Times(1);

    scheduler_->ScheduleJob(*job);
    EXPECT_TRUE(scheduler_->RemoveJob(job->getJobid())) << "Call should succeed if job exist";
    delete job;
}

TEST_F(JobSchedStaticWL, RemoveNonExistingJobShouldFail) {

    auto job = CreateJob("job", 1, 1000);
    std::vector<std::string> osts_touched_by_job = CreateSingleOstList();
    job->setOsts(osts_touched_by_job);

    EXPECT_CALL(*mock_job_monitor_, UnregisterJob(_)).Times(0);

    EXPECT_FALSE(scheduler_->RemoveJob(job->getJobid())) << "Call should fail if job doesn't exist";
    delete job;
}