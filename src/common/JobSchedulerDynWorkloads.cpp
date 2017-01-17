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

#include "JobSchedulerDynWorkloads.h"

namespace common {
JobSchedulerDynWorkloads::JobSchedulerDynWorkloads(std::shared_ptr<ScheduleState> &schedule,
                                                   std::shared_ptr<JobMonitor> job_monitor,
                                                   std::shared_ptr<ClusterState> cluster_state,
                                                   std::shared_ptr<Lustre> lustre)
        : schedule_(schedule), job_monitor_(job_monitor), cluster_state_(cluster_state), lustre_(lustre) {}

bool JobSchedulerDynWorkloads::ScheduleJob(common::Job &job) {

    std::lock_guard<std::mutex> lck(scheduler_mut_);

    for (auto ost : job.getOsts()) {
        // get the resource utilization for the timeframe of the job

        uint32_t max_ost_mb_sec;
        if (!GetMaxLoadInTimeInterval(ost, job.GetStartTime(), job.GetEndTime(), &max_ost_mb_sec)) {
            return false;   // invalid OST? TODO: report an error here
        }

        OSTWorkload node_state;
        if (!cluster_state_->getOstState(ost, &node_state)) {
            return false;
        }

        if (!AreEnoughResAvail(job, max_ost_mb_sec, node_state)) {
            return false;   // not enough resources avail? TODO: report it somewhere
        }
    }

    if (!schedule_->AddJob(job.getJobid(), job, job.getOsts())) {
        return false;
    }

    if (!job_monitor_->RegisterJob(job)) {
        schedule_->RemoveJob(job.getJobid());
        return false;
    }

    return true;
}

bool JobSchedulerDynWorkloads::AreEnoughResAvail(const Job &job, uint32_t max_ost_mb_sec,
                                                 const OSTWorkload &node_state) const {
    return ((max_ost_mb_sec + job.getMin_read_throughput_MB()) <= lustre_->RPCsToMBs(node_state.maxRpcSec));
}

bool JobSchedulerDynWorkloads::RemoveJob(const std::string &jobid) {

    Job::JobState state;
    if (JobDoesNotExist(jobid, state)) { // does the job exist at all?
        return false;
    }

    if (!job_monitor_->UnregisterJob(jobid)) {
        // TODO: give a warning here
    }
    return schedule_->RemoveJob(jobid);
}

bool JobSchedulerDynWorkloads::JobDoesNotExist(const std::string &jobid, Job::JobState &state) const {
    return !schedule_->GetJobStatus(jobid, &state);
}

bool JobSchedulerDynWorkloads::GetMaxLoadInTimeInterval(std::string ost, std::chrono::system_clock::time_point start,
                                                        std::chrono::system_clock::time_point end,
                                                        uint32_t *maxLoadMBSec) {

    OSTWorkload node_state;
    if (!cluster_state_->getOstState(ost, &node_state)) {
        return false;
    }

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

bool JobSchedulerDynWorkloads::Init() {
    if ((schedule_ == nullptr) || (job_monitor_ == nullptr) || (lustre_ == nullptr)) {
        return false;
    }

    return true;
}

bool JobSchedulerDynWorkloads::Teardown() {
    return true; // noop
}


}


