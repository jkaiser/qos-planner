//
// Created by jkaiser on 15.03.16.
//

#include "ScheduleState.h"

namespace common {

bool MemoryScheduleState::Init() {
    std::lock_guard<std::mutex> lck(schedule_mut);
    return true;
}

bool MemoryScheduleState::TearDown() {
    std::lock_guard<std::mutex> lck(schedule_mut);
    return true;
}

const std::map<std::string, std::list<Job*>> *MemoryScheduleState::GetClusterState() {

    std::lock_guard<std::mutex> lck(schedule_mut);
    auto new_sched = new std::map<std::string, std::list<Job*>>();

    for (auto &&it : schedule) {
        (*new_sched)[it.first] = std::list<Job*>(it.second);
    }

    return new_sched;
}

const std::list<Job *> *MemoryScheduleState::GetOSTState(const std::string &ost) {
    std::lock_guard<std::mutex> lck(schedule_mut);
    std::map<std::string, std::list<Job*>>::const_iterator it = schedule.find(ost);
    if (it == schedule.end()) {
        return nullptr;
    }
    return &it->second;
}

void MemoryScheduleState::Reset() {
    std::lock_guard<std::mutex> lck(schedule_mut);

    for (auto &&job : jobs) {
        delete job.second;
    }
    schedule.clear();
    jobs.clear();
}

bool MemoryScheduleState::AddJob(const std::string &jobid, const Job &job, const std::vector<std::string> &osts) {
    std::lock_guard<std::mutex> lck(schedule_mut);

    if (jobs.find(jobid) != jobs.end()) {
        return false;
    }

    auto to_insert_job = new Job(job);

    for (auto &&ost : osts) {
        if (schedule.find(ost) == schedule.end()) {
            schedule[ost] = std::list<Job*>();
        }

        std::list<Job*> *ost_sched = &schedule[ost];
        auto it = ost_sched->begin();
        for (; it != ost_sched->end(); it++) {
            if ((*it)->GetStartTime() > job.GetStartTime()) {
                break;
            }
        }

        ost_sched->insert(it, to_insert_job);
    }

    jobs[jobid] = to_insert_job;
    return true;
}


bool MemoryScheduleState::RemoveJob(const std::string &jobid) {
    std::lock_guard<std::mutex> lck(schedule_mut);

    if (jobs.find(jobid) == jobs.end()) {
        return false;
    }

    Job *job = jobs[jobid];
    jobs.erase(jobid);


    for (auto ost : job->getOsts()) {

        std::list<Job*> ost_job_list = schedule[ost];
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
    std::lock_guard<std::mutex> lck(schedule_mut);

    auto it = jobs.find(jobid);
    if (it == jobs.end()) {
        return false;
    }

    it->second->setState(new_state);
    return true;
}

std::map<std::string, Job *> *MemoryScheduleState::GetAllJobs() {
    std::lock_guard<std::mutex> lck(schedule_mut);

    auto new_map = new std::map<std::string, Job*>();

    for (auto &&it : jobs){
        (*new_map)[it.first] = new Job(*it.second);
    }

    return new_map;
}

bool MemoryScheduleState::GetJobThroughput(std::string jobid, uint32_t *throughput) {
    std::lock_guard<std::mutex> lck(schedule_mut);
    auto it = jobs.find(jobid);
    if (it == jobs.end()) {
        return false;
    }

    *throughput = it->second->getMin_read_throughput_MB();
    return true;
}

bool MemoryScheduleState::GetJobStatus(const std::string jobid, Job::JobState *state) {
    std::lock_guard<std::mutex> lck(schedule_mut);
    auto it = jobs.find(jobid);
    if (it == jobs.end()) {
        return false;
    }

    *state = it->second->getState();
    return true;
}

bool MemoryScheduleState::GetJobEnd(const std::string jobid, std::chrono::system_clock::time_point *tend) {
    std::lock_guard<std::mutex> lck(schedule_mut);
    auto it = jobs.find(jobid);
    if (it == jobs.end()) {
        return false;
    }

    *tend = it->second->GetEndTime();
    return true;
}



}
