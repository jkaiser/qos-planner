//
// Created by jkaiser on 17.03.16.
//

#include "JobMonitor.h"

#include <spdlog/spdlog.h>

namespace common {

JobMonitor::JobMonitor() {}

JobMonitor::JobMonitor(std::shared_ptr<common::ScheduleState> st, std::shared_ptr<RuleManager> rule_manager) :
        monitor_thread_started_(false),
        scheduleState_(st),
        rule_manager_(rule_manager) {
    JobMonitor(st, rule_manager, 5);
}

JobMonitor::JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<RuleManager> rule_manager, uint32_t waiting_time_sec) :
        monitor_thread_started_(false),
        scheduleState_(st),
        rule_manager_(rule_manager),
        waiting_time_sec_(waiting_time_sec) {}


bool JobMonitor::RegisterJob(const common::Job &job) {
    std::lock_guard<std::mutex> lock(job_priority_queue_mutex_);
    JobPriorityQueue::WaitingItem *wt = new JobPriorityQueue::WaitingItem(job.getJobid(), job.GetStartTime(),
    Job::JobEvent::JOBSTART);
    job_priority_queue_.Push(wt);
    return true;
}
void JobMonitor::Handle(const std::string &jobid, Job::JobEvent event) {
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex_);
    while (in_flight_jobs_.find(jobid) != in_flight_jobs_.end()) {
        in_flight_jobs_cv_.wait(lock);
    }
    in_flight_jobs_.insert(jobid);
    lock.unlock();

    switch (event) {
        case Job::JobEvent::JOBSTART:
            StartJob(jobid);
            break;
        case Job::JobEvent::JOBSTOP:
            StopJob(jobid);
            scheduleState_->RemoveJob(jobid);
            break;
        default:
            // Error
            // TODO: announce this error
            break;
    }

    lock.lock();
    in_flight_jobs_.erase(jobid);
    in_flight_jobs_cv_.notify_all();
    lock.unlock();
}

bool JobMonitor::Init() {
    if ((scheduleState_ == nullptr) || (rule_manager_ == nullptr)) {
        return false;
    }

    FillWithExistingJobs();

    if (not monitor_thread_started_) {
        monitor_thread_ = std::thread(&JobMonitor::Monitor, this);
        monitor_thread_started_ = true;
    } else { // something's wrong. Was it initialized before?
        return false;
    }

    return true;
}

void JobMonitor::FillWithExistingJobs() {
    // We only consider jobs that were touched by the monitor itself -> state == SCHEDULED or state == ACTIVE
    std::map<std::string, Job *> *existing_jobs = scheduleState_->GetAllJobs();

    std::unique_lock<std::mutex> lock(job_priority_queue_mutex_);
    for (auto job : *existing_jobs) {
        JobPriorityQueue::WaitingItem *wt;
        switch (job.second->getState()) {

            case Job::SCHEDULED:
                wt = new JobPriorityQueue::WaitingItem(job.second->getJobid(), job.second->GetStartTime(),
                Job::JOBSTART);
                job_priority_queue_.Push(wt);
                break;

            case Job::ACTIVE:
                wt = new JobPriorityQueue::WaitingItem(job.second->getJobid(), job.second->GetEndTime(), Job::JOBSTOP);
                job_priority_queue_.Push(wt);
                break;

            case Job::INITIALIZED:
                break;    // job is there, but never was scheduled -> to do
            case Job::DONE:
                break;           // job is finished -> nothing to do
        }
        delete job.second;
    }
    existing_jobs->clear();
    delete existing_jobs;
    lock.unlock();
}


bool JobMonitor::TearDown() {

    std::unique_lock<std::mutex> lock(job_priority_queue_mutex_);
    monitor_thread_exit_flag_ = true;
    while (monitor_thread_is_active_) {
        monitor_thread_finish_cv_.wait(lock);
    }
    lock.unlock();


    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    return true;
}

void JobMonitor::Monitor() {

    std::unique_lock<std::mutex> lk(job_priority_queue_mutex_);
    monitor_thread_is_active_ = true;
    monitor_thread_finish_cv_.notify_all();
    lk.unlock();

    while (!monitor_thread_exit_flag_) {
        lk.lock();
        while (isThereAReadyJob()) {
            JobPriorityQueue::WaitingItem *wt = job_priority_queue_.Pop();
            lk.unlock();

            Handle(wt->jobid, wt->eventType);
            delete wt;
            lk.lock();
        }

        job_priority_queue_cv_.wait_for(lk, std::chrono::seconds(waiting_time_sec_));
        lk.unlock();
    }

    lk.lock();
    monitor_thread_is_active_ = false;
    monitor_thread_finish_cv_.notify_all();
    lk.unlock();

}

bool JobMonitor::isThereAReadyJob() const {
    return (job_priority_queue_.Peek() != nullptr) &&
    (job_priority_queue_.Peek()->time_of_event < std::chrono::system_clock::now());
}


bool JobMonitor::StartJob(const std::string &jobid) {

    spdlog::get("console")->debug("starting job {}", jobid);

    Job::JobState job_state;
    if (!scheduleState_->GetJobStatus(jobid, &job_state)) {   // job was removed in the meantime
        spdlog::get("console")->debug("job was removed in the meantime");
        return false;
    } else if (job_state != Job::SCHEDULED) {   // job was unregistered/removed in the meantime
        spdlog::get("console")->debug("job was unregistered in the meantime");
        return false;
    }

    spdlog::get("console")->debug("setting up NRS rules");
    // 2) set the NRS settings
    if (!SetNrsRules(jobid)) {
        spdlog::get("console")->error("couldn't set NRS settings for job {}!", jobid);
        return false;
    };

    spdlog::get("console")->debug("update schedule status");
    // 3) update job status to ACTIVE
    if (!scheduleState_->UpdateJob(jobid, Job::ACTIVE)) {
        return false;
    }

    // 4) setup new waiting mechanism
    std::chrono::system_clock::time_point tend;
    if (!scheduleState_->GetJobEnd(jobid, &tend)) {
        return false;
    }
    std::unique_lock<std::mutex> lock(job_priority_queue_mutex_);
    JobPriorityQueue::WaitingItem *wt = new JobPriorityQueue::WaitingItem(jobid, tend, Job::JobEvent::JOBSTOP);
    job_priority_queue_.Push(wt);
    job_priority_queue_cv_.notify_all();
    lock.unlock();

    return true;
}

bool JobMonitor::StopJob(const std::string &jobid) {
    spdlog::get("console")->debug("jobmon: stop job {}", jobid);

    Job::JobState job_state;
    if (!scheduleState_->GetJobStatus(jobid, &job_state)) {   // job was removed in the meantime
        spdlog::get("console")->error("job was removed in the meantime");
        return false;
    } else if (job_state != Job::ACTIVE) {   // job was unregistered/removed in the meantime
        spdlog::get("console")->error("job was unregistered in the meantime");
        return false;
    }

    // set the NRS settings
    if (!rule_manager_->RemoveRules(jobid)) {
        spdlog::get("console")->warn("couldn't remove TBF rule for job {}", jobid);
    }

    // update job status to DONE
    return scheduleState_->UpdateJob(jobid, Job::DONE);

}

bool JobMonitor::UnregisterJob(const Job &job) {

    BlockJob(job);
    RemoveJobFromPrioQueue(job);

    Job::JobState job_state;
    if (!scheduleState_->GetJobStatus(job.getJobid(), &job_state)) {   // job was removed in the meantime
        UnblockJob(job);
        return true;
    }

    if (job_state == Job::ACTIVE) {
        // job is active -> a rule for this one was set -> stop it
        if (!StopJob(job.getJobid())) {
            UnblockJob(job);
            return false;
            //TODO: Give an error message here
        }
    }

    UnblockJob(job);
    return true;
}

void JobMonitor::BlockJob(const Job &job) {
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex_);
    while (in_flight_jobs_.find(job.getJobid()) != in_flight_jobs_.end()) {
        in_flight_jobs_cv_.wait(lock);
    }
    in_flight_jobs_.insert(job.getJobid());
    lock.unlock();
}

void JobMonitor::UnblockJob(const Job &job) {
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex_);
    in_flight_jobs_.erase(job.getJobid());
    in_flight_jobs_cv_.notify_all();
    lock.unlock();
}

void JobMonitor::RemoveJobFromPrioQueue(const Job &job) {
    std::unique_lock<std::mutex> lk(job_priority_queue_mutex_);
    if (!job_priority_queue_.Remove(job.getJobid())) {
        spdlog::get("console")->warn("couldn't remove job from prio. queue {}");
        //TODO: give a warning here. This should not happen but isn't critical here since we remove anyway.
    }
    job_priority_queue_cv_.notify_all();
    lk.unlock();
}

bool JobMonitor::SetNrsRules(const std::string &jobid) {
    uint32_t requested_throughput;
    if (!scheduleState_->GetJobThroughput(jobid, &requested_throughput)) {
        spdlog::get("console")->error("schedule doesn't contain job {}!", jobid);
        return false;
    }

    std::vector<std::string> osts_ids;
    scheduleState_->GetJobOstIds(jobid, osts_ids);

    if (!rule_manager_->SetRules(osts_ids, jobid, requested_throughput)) {
        spdlog::get("console")->error("failed to set NRS rules for job {}!", jobid);
        return false;
    }

    return true;
}


}


