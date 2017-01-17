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

#include "MemoryScheduleState.h"
#include <spdlog/spdlog.h>


namespace common {

bool MemoryScheduleState::Init() {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    return true;
}

bool MemoryScheduleState::TearDown() {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    return true;
}

const std::map<std::string, std::list<Job*>> *MemoryScheduleState::GetClusterState() {

    std::lock_guard<std::mutex> lck(schedule_mut_);
    auto new_sched = new std::map<std::string, std::list<Job*>>();

    for (auto &&it : schedule_) {
        (*new_sched)[it.first] = std::list<Job*>(it.second);
    }

    return new_sched;
}

const std::list<Job *> *MemoryScheduleState::GetOSTState(const std::string &ost) {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    std::map<std::string, std::list<Job*>>::const_iterator it = schedule_.find(ost);
    if (it == schedule_.end()) {
        return nullptr;
    }
    return &it->second;
}

void MemoryScheduleState::Reset() {
    std::lock_guard<std::mutex> lck(schedule_mut_);

    for (auto &&job : jobs_) {
        delete job.second;
    }
    schedule_.clear();
    jobs_.clear();
}

bool MemoryScheduleState::AddJob(const std::string &jobid, const Job &job, const std::vector<std::string> &osts) {
    std::lock_guard<std::mutex> lck(schedule_mut_);

    spdlog::get("console")->debug("schedule: add job {}", jobid);
    if (jobs_.find(jobid) != jobs_.end()) {
        spdlog::get("console")->error("job already exists!");
        return false;
    }

    auto to_insert_job = new Job(job);

    for (auto &&ost : osts) {
        if (schedule_.find(ost) == schedule_.end()) {
            schedule_[ost] = std::list<Job*>();
        }

        std::list<Job*> *ost_sched = &schedule_[ost];
        auto it = ost_sched->begin();
        for (; it != ost_sched->end(); it++) {
            if ((*it)->GetStartTime() > job.GetStartTime()) {
                break;
            }
        }

        ost_sched->insert(it, to_insert_job);
    }

    jobs_[jobid] = to_insert_job;
    return true;
}


bool MemoryScheduleState::RemoveJob(const std::string &jobid) {
    std::lock_guard<std::mutex> lck(schedule_mut_);

    spdlog::get("console")->debug("schedule: remove job {}", jobid);

    if (jobs_.find(jobid) == jobs_.end()) {
        spdlog::get("console")->error("schedule: job doesn't exist");
        return false;
    }

    Job *job = jobs_[jobid];
    jobs_.erase(jobid);


    for (auto ost : job->getOsts()) {

        std::list<Job*> ost_job_list = schedule_[ost];
        for (std::list<Job*>::iterator it = ost_job_list.begin(); it !=  ost_job_list.end(); it++) {

            if ((*it)->getJobid().compare(jobid) == 0) {
                ost_job_list.erase(it);
                break;
            }
        }
    }

    return true;
}

bool MemoryScheduleState::UpdateJob(std::string jobid, Job::JobState new_state) {
    std::lock_guard<std::mutex> lck(schedule_mut_);

    spdlog::get("console")->debug("schedule: update job {} to {}", jobid, Job::JobStateToString(new_state));

    auto it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return false;
    }

    it->second->setState(new_state);
    return true;
}

std::map<std::string, Job *> *MemoryScheduleState::GetAllJobs() {
    std::lock_guard<std::mutex> lck(schedule_mut_);

    auto new_map = new std::map<std::string, Job*>();

    for (auto &&it : jobs_){
        (*new_map)[it.first] = new Job(*it.second);
    }

    return new_map;
}

bool MemoryScheduleState::GetJobThroughput(std::string jobid, uint32_t *throughput) {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    auto it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return false;
    }

    *throughput = it->second->getMin_read_throughput_MB();
    return true;
}

bool MemoryScheduleState::GetJobStatus(const std::string jobid, Job::JobState *state) {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    auto it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        spdlog::get("console")->error("schedule: requested nonexisting job");
        return false;
    }

    *state = it->second->getState();
    return true;
}

bool MemoryScheduleState::GetJobEnd(const std::string jobid, std::chrono::system_clock::time_point *tend) {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    auto it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return false;
    }

    *tend = it->second->GetEndTime();
    return true;
}

bool MemoryScheduleState::GetJobOstIds(const std::string &jobid, std::vector<std::string> &osts_out) {
    std::lock_guard<std::mutex> lck(schedule_mut_);
    auto it = jobs_.find(jobid);
    if (it == jobs_.end()) {
        return false;
    }

    osts_out.insert(osts_out.end(), it->second->getOsts().begin(), it->second->getOsts().end());
    return true;
}
}
