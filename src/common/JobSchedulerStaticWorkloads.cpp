//
// Created by Jürgen Kaiser on 01.10.16.
//

#include "JobSchedulerStaticWorkloads.h"
#include "OSTLimitConfigParser.h"

#include <fstream>

#include <spdlog/spdlog.h>

namespace common {

JobSchedulerStaticWorkloads::JobSchedulerStaticWorkloads(std::shared_ptr<ScheduleState> &schedule,
                                                   std::shared_ptr<JobMonitor> job_monitor,
                                                   std::shared_ptr<Lustre> lustre,
                                                   std::string &ost_limits_file)
        : schedule(schedule), job_monitor(job_monitor), lustre(lustre), ost_limits_file(ost_limits_file) {}

bool JobSchedulerStaticWorkloads::ScheduleJob(common::Job &job) {
    std::lock_guard<std::mutex> lck(scheduler_mut);

    for (auto ost : job.getOsts()) {
        uint32_t max_ost_mb_sec;
        if (!GetMaxLoadInTimeInterval(ost, job.GetStartTime(), job.GetEndTime(), &max_ost_mb_sec)) {
            return false;   // invalid OST? TODO: report an error here
        }

        if (!AreEnoughResAvail(job, ost, max_ost_mb_sec))  {
            return false;   // not enough resources avail? TODO: report it somewhere
        }
    }

    if (!schedule->AddJob(job.getJobid(), job, job.getOsts())) {
        return false;
    }

    if (!job_monitor->RegisterJob(job)) {
        schedule->RemoveJob(job.getJobid());
        return false;
    }

    return true;
}

bool JobSchedulerStaticWorkloads::AreEnoughResAvail(const Job &job, const std::string &ost, uint32_t max_ost_mb_sec) {
    std::map<std::string, float>::iterator it = osts_max_mbs_limits_.find(ost);
    if (it == osts_max_mbs_limits_.end()) {
        return false; // no information about that ost
    }

    return (max_ost_mb_sec + job.getMin_read_throughput_MB()) <= it->second;
}

bool JobSchedulerStaticWorkloads::GetMaxLoadInTimeInterval(std::string ost, std::chrono::system_clock::time_point start,
                                                        std::chrono::system_clock::time_point end, uint32_t *maxLoadMBSec) {

    const std::list<Job *> *job_list = schedule->GetOSTState(ost);
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

    if (!job_monitor->UnregisterJob(jobid)) {
        // TODO: give a warning here
    }
    return schedule->RemoveJob(jobid);
}

bool JobSchedulerStaticWorkloads::JobDoesNotExist(const std::string &jobid, Job::JobState &state) const { return !schedule->GetJobStatus(jobid, &state); }

void JobSchedulerStaticWorkloads::UpdateLimits(const std::map<std::string, uint32_t> &new_limits) {
    osts_max_mbs_limits_.clear();
    osts_max_mbs_limits_.insert(new_limits.begin(), new_limits.end());
}

bool JobSchedulerStaticWorkloads::Init() {

    spdlog::get("console")->info("init scheduler");
    if (!ost_limits_file.empty()) {
        std::ifstream is(ost_limits_file, std::ifstream::in);
        if (!is.is_open()) {
            spdlog::get("console")->critical("ost limits file \"{}\" doesn't exist!", ost_limits_file);
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