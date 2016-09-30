//
// Created by jkaiser on 22.03.16.
//

#include "Scheduler.h"

namespace common {
Scheduler::Scheduler(std::shared_ptr<ScheduleState> &schedule,
                     std::shared_ptr<JobMonitor> job_monitor,
                     std::shared_ptr<ClusterState> cluster_state,
                     std::shared_ptr<Lustre> lustre)
            : schedule(schedule), job_monitor(job_monitor), cluster_state(cluster_state), lustre(lustre) {
        this->job_monitor = job_monitor;
}

bool Scheduler::ScheduleJob(common::Job &job) {

    std::lock_guard<std::mutex> lck(scheduler_mut);

    for (auto ost : job.getOsts()) {
        // get the resource utilization for the timeframe of the job

        uint32_t max_ost_mb_sec;
        if (!GetMaxLoadInTimeInterval(ost, job.GetStartTime(), job.GetEndTime(), &max_ost_mb_sec)) {
            return false;   // invalid OST? TODO: report an error here
        }

        OSTWorkload node_state;
        if (!cluster_state->getOstState(ost, &node_state)) {
            return false;
        }

        if (!AreEnoughResAvail(job, max_ost_mb_sec, node_state))  {
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

    bool Scheduler::AreEnoughResAvail(const Job &job, uint32_t max_ost_mb_sec, const OSTWorkload &node_state) const {
        return ((max_ost_mb_sec + job.getMin_read_throughput_MB()) <= lustre->RPCsToMBs(node_state.maxRpcSec));
    }

    bool Scheduler::RemoveJob(const std::string &jobid) {

    Job::JobState state;
    if (JobDoesNotExist(jobid, state)) { // does the job exist at all?
        return false;
    }

    if (!job_monitor->UnregisterJob(jobid)) {
        // TODO: give a warning here
    }
    return schedule->RemoveJob(jobid);
}

    bool Scheduler::JobDoesNotExist(const std::string &jobid, Job::JobState &state) const { return !schedule->GetJobStatus(jobid, &state); }

    bool Scheduler::GetMaxLoadInTimeInterval(std::string ost, std::chrono::system_clock::time_point start,
                                         std::chrono::system_clock::time_point end, uint32_t *maxLoadMBSec) {

    OSTWorkload node_state;
    if (!cluster_state->getOstState(ost, &node_state)) {
        return false;
    }

    const std::list<Job *> *job_list = schedule->GetOSTState(ost);
    if (job_list == nullptr) {
        *maxLoadMBSec = 0;
        return true;
    }


    uint32_t max_load_mb_sec;
    for (auto job : *job_list) {

        if ((job->GetStartTime() > end) || (job->GetEndTime() < start)) {   // is the job outside of the time interval?
           continue;
        }

        max_load_mb_sec += job->getMin_read_throughput_MB();
    }
    return true;
}

bool Scheduler::Init() {
    if (schedule == nullptr) {
        return false;
    }

    if (job_monitor == nullptr) {
        return false;
    }
    if (lustre == nullptr) {
        return false;
    }
    return true;
}

bool Scheduler::TearDown() {
    return true; // noop
}


}


