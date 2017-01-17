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

#include "JobSchedulerStaticWorkloads.h"
#include "OSTLimitConfigParser.h"

#include <fstream>

#include <spdlog/spdlog.h>

namespace common {

JobSchedulerStaticWorkloads::JobSchedulerStaticWorkloads(std::shared_ptr<ScheduleState> &schedule,
                                                   std::shared_ptr<JobMonitor> job_monitor,
                                                   std::shared_ptr<Lustre> lustre,
                                                   std::string &ost_limits_file)
        : schedule_(schedule), job_monitor_(job_monitor), lustre_(lustre), ost_limits_file_(ost_limits_file) {}

bool JobSchedulerStaticWorkloads::ScheduleJob(common::Job &job) {
    std::lock_guard<std::mutex> lck(scheduler_mut_);

    for (auto ost : job.getOsts()) {
        uint32_t max_ost_mb_sec;
        if (!GetMaxLoadInTimeInterval(ost, job.GetStartTime(), job.GetEndTime(), &max_ost_mb_sec)) {
            spdlog::get("console")->error("couldn't compute max load for ost {}", ost);
            return false;   // invalid OST?
        }

        if (!AreEnoughResAvail(job, ost, max_ost_mb_sec))  {
            spdlog::get("console")->debug("not enough resources for job");
            return false;   // not enough resources avail?
        }
    }

    job.setState(Job::SCHEDULED);
    if (!schedule_->AddJob(job.getJobid(), job, job.getOsts())) {
        return false;
    }

    if (!job_monitor_->RegisterJob(job)) {
        spdlog::get("console")->error("failed to register job");
        schedule_->RemoveJob(job.getJobid());
        return false;
    }

    return true;
}

bool JobSchedulerStaticWorkloads::AreEnoughResAvail(const Job &job, const std::string &ost, uint32_t max_ost_mb_sec) {
    std::map<std::string, float>::iterator it = osts_max_mbs_limits_.find(ost);
    if (it == osts_max_mbs_limits_.end()) {
        spdlog::get("console")->error("no information about the ost '{}'", ost);
        return false;
    }

    return (max_ost_mb_sec + job.getMin_read_throughput_MB()) <= it->second;
}

bool JobSchedulerStaticWorkloads::GetMaxLoadInTimeInterval(std::string ost, std::chrono::system_clock::time_point start,
                                                        std::chrono::system_clock::time_point end, uint32_t *maxLoadMBSec) {

    const std::list<Job *> *job_list = schedule_->GetOSTState(ost);
    if (job_list == nullptr) {
        *maxLoadMBSec = 0;
        return true;
    }

    uint32_t max_load_mb_sec = 0;
    for (auto job : *job_list) {

        if ((job->GetStartTime() > end) || (job->GetEndTime() < start)) {   // is the job outside of the time interval?
            continue;
        }

        max_load_mb_sec += job->getMin_read_throughput_MB();
    }
    return true;
}

bool JobSchedulerStaticWorkloads::RemoveJob(const std::string &jobid) {
    Job::JobState state;
    if (JobDoesNotExist(jobid, state)) {
        return false;
    }

    if (!job_monitor_->UnregisterJob(jobid)) {
        // TODO: give a warning here
    }
    return schedule_->RemoveJob(jobid);
}

bool JobSchedulerStaticWorkloads::JobDoesNotExist(const std::string &jobid, Job::JobState &state) const { return !schedule_->GetJobStatus(jobid, &state); }

void JobSchedulerStaticWorkloads::UpdateLimits(const std::map<std::string, uint32_t> &new_limits) {
    osts_max_mbs_limits_.clear();
    osts_max_mbs_limits_.insert(new_limits.begin(), new_limits.end());
}

bool JobSchedulerStaticWorkloads::Init() {

    spdlog::get("console")->info("init scheduler");
    if (!ost_limits_file_.empty()) {
        std::ifstream is(ost_limits_file_, std::ifstream::in);
        if (!is.is_open()) {
            spdlog::get("console")->critical("ost limits file \"{}\" doesn't exist!", ost_limits_file_);
            return false;
        }

        OSTLimitConfigParser p;
        if (!p.Parse(is)) {
            return false;
        }

        osts_max_mbs_limits_ = p.GetLimits();
    }
    return true;
}

bool JobSchedulerStaticWorkloads::Teardown() {
    return true;
}

}