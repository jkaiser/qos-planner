//
// Created by jkaiser on 17.03.16.
//

#include "JobMonitor.h"

namespace common {

JobMonitor::JobMonitor(std::shared_ptr<common::ScheduleState> st, std::shared_ptr<Lustre> lustre) :
        monitor_thread_started(false),
        scheduleState(st),
        lustre(lustre) {
    JobMonitor(st, lustre, 5);
}

JobMonitor::JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<Lustre> lustre, uint32_t waiting_time_sec) :
        monitor_thread_started(false),
        scheduleState(st),
        lustre(lustre),
        waiting_time_sec(waiting_time_sec) {}

bool JobMonitor::RegisterJob(const common::Job &job) {
    std::lock_guard<std::mutex> lock(job_priority_queue_mutex);
    JobPriorityQueue::WaitingItem *wt = new JobPriorityQueue::WaitingItem(job.getJobid(), job.GetStartTime(), Job::JobEvent::JOBSTART);
    job_priority_queue.Push(wt);
    return true;
}

void JobMonitor::Handle(const std::string &jobid, Job::JobEvent event) {
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex);
    while (in_flight_jobs.find(jobid) != in_flight_jobs.end()) {
        in_flight_jobs_cv.wait(lock);
    }
    in_flight_jobs.insert(jobid);
    lock.unlock();

    switch(event) {
        case Job::JobEvent::JOBSTART:
            StartJob(jobid);
            break;
        case Job::JobEvent::JOBSTOP:
            StopJob(jobid);
            break;
        default:
            // Error
            // TODO: announce this error
            break;
    }

    lock.lock();
    in_flight_jobs.erase(jobid);
    in_flight_jobs_cv.notify_all();
    lock.unlock();
}

bool JobMonitor::Init() {
    if ((scheduleState == nullptr) || (lustre == nullptr)) {
        return false;
    }

    FillWithExistingJobs();

    if (not monitor_thread_started) {
        monitor_thread = std::thread(&JobMonitor::Monitor, this);
        monitor_thread_started = true;
    } else { // something's wrong. Was it initialized before?
        return false;
    }

    return true;
}

    void JobMonitor::FillWithExistingJobs() {
        // We only consider jobs that were touched by the monitor itself -> state == SCHEDULED or state == ACTIVE
        std::map<std::string, Job*> *existing_jobs = scheduleState->GetAllJobs();

        std::unique_lock<std::mutex> lock(job_priority_queue_mutex);
        for (auto job : *existing_jobs) {
            JobPriorityQueue::WaitingItem *wt;
            switch (job.second->getState()) {

                case Job::SCHEDULED:
                    wt = new JobPriorityQueue::WaitingItem(job.second->getJobid(), job.second->GetStartTime(), Job::JOBSTART);
                    job_priority_queue.Push(wt);
                    break;

                case Job::ACTIVE:
                    wt = new JobPriorityQueue::WaitingItem(job.second->getJobid(), job.second->GetEndTime(), Job::JOBSTOP);
                    job_priority_queue.Push(wt);
                    break;

                case Job::INITIALIZED:break;    // job is there, but never was scheduled -> to do
                case Job::DONE:break;           // job is finished -> nothing to do
            }
            delete job.second;
        }
        existing_jobs->clear();
        delete existing_jobs;
        lock.unlock();
    }


    bool JobMonitor::TearDown() {

    std::unique_lock<std::mutex> lock(job_priority_queue_mutex);
    monitor_thread_exit_flag = true;
    while (monitor_thread_is_active) {
        monitor_thread_finish_cv.wait(lock);
    }
    lock.unlock();


    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }

    return true;
}

void JobMonitor::Monitor() {

    std::unique_lock<std::mutex> lk(job_priority_queue_mutex);
    monitor_thread_is_active = true;
    monitor_thread_finish_cv.notify_all();
    lk.unlock();

    while (!monitor_thread_exit_flag) {
        lk.lock();
        while(isThereAReadyJob()) {
            JobPriorityQueue::WaitingItem *wt = job_priority_queue.Pop();
            lk.unlock();

            Handle(wt->jobid, wt->eventType);
            delete wt;
            lk.lock();
        }

        job_priority_queue_cv.wait_for(lk, std::chrono::seconds(waiting_time_sec));
        lk.unlock();
    }

    lk.lock();
    monitor_thread_is_active = false;
    monitor_thread_finish_cv.notify_all();
    lk.unlock();

}

    bool JobMonitor::isThereAReadyJob() const {
        return (job_priority_queue.Peek() != nullptr ) &&
               (job_priority_queue.Peek()->time_of_event < std::__1::chrono::system_clock::now());
    }


    bool JobMonitor::StartJob(const std::string &jobid) {

    Job::JobState job_state;
    if (!scheduleState->GetJobStatus(jobid, &job_state)) {   // job was removed in the meantime
        return false;
    } else if (job_state != Job::SCHEDULED) {   // job was unregistered/removed in the meantime
        return false;
    }

    // 2) set the NRS settings
    uint32_t requested_throughput;
    if(!scheduleState->GetJobThroughput(jobid, &requested_throughput)) {
        return false;
    }


    uint32_t rpc_rate = lustre->MBsToRPCs(requested_throughput);
    if (!lustre->StartJobTbfRule(jobid, jobid + lustre_tbf_rule_postfix, rpc_rate)) {
        return false;
    }


    // 3) update job status to ACTIVE
    if (!scheduleState->UpdateJob(jobid, Job::ACTIVE)) {
        return false;
    }

    // 4) setup new waiting mechanism
    std::chrono::system_clock::time_point tend;
    if (!scheduleState->GetJobEnd(jobid, &tend)) {
        return false;
    }
    std::unique_lock<std::mutex> lock(job_priority_queue_mutex);
    JobPriorityQueue::WaitingItem *wt = new JobPriorityQueue::WaitingItem(jobid, tend, Job::JobEvent::JOBSTOP);
    job_priority_queue.Push(wt);
    job_priority_queue_cv.notify_all();
    lock.unlock();

    return true;
}

bool JobMonitor::StopJob(const std::string &jobid) {

    Job::JobState job_state;
    if (!scheduleState->GetJobStatus(jobid, &job_state)) {   // job was removed in the meantime
        return false;
    } else if (job_state != Job::ACTIVE) {   // job was unregistered/removed in the meantime
        return false;
    }

    // set the NRS settings
    if(!lustre->StopJobTbfRule(jobid, jobid + lustre_tbf_rule_postfix)) {
        //TODO: give a warning here. This can lead to open permanent open NRS settings in Lustre!
    }

    // update job status to DONE
    return scheduleState->UpdateJob(jobid, Job::DONE);

}

bool JobMonitor::UnregisterJob(const Job &job) {


    // 1) block the job
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex);
    while (in_flight_jobs.find(job.getJobid()) != in_flight_jobs.end()) {
        in_flight_jobs_cv.wait(lock);
    }
    in_flight_jobs.insert(job.getJobid());
    lock.unlock();


    // 2) prevent the thread from doing anything new
    std::unique_lock<std::mutex> lk(job_priority_queue_mutex);
    if (!job_priority_queue.Remove(job.getJobid())) {
        //TODO: give a warning here. This should not happen but isn't critical here since we remove anyway.
    }
    job_priority_queue_cv.notify_all();
    lk.unlock();

    // 3) check if we have to do more
    Job::JobState job_state;
    if (!scheduleState->GetJobStatus(job.getJobid(), &job_state)) {   // job was removed in the meantime
        lock.lock();
        in_flight_jobs.erase(job.getJobid());
        in_flight_jobs_cv.notify_all();
        lock.unlock();
        return true;
    }

    if (job_state != Job::ACTIVE) {   // job isn't running -> no rule was set -> nothing to do
        lock.lock();
        in_flight_jobs.erase(job.getJobid());
        in_flight_jobs_cv.notify_all();
        lock.unlock();
        return true;
    }

    // 4) job is active -> a rule for this one was set -> stop it
    bool ret_val = true;
    if (!StopJob(job.getJobid())) {
        ret_val = false;
        //TODO: Give an error message here
    }

    // 5) unblock the job
    lock.lock();
    in_flight_jobs.erase(job.getJobid());
    in_flight_jobs_cv.notify_all();
    lock.unlock();

    return ret_val;
}

JobMonitor::JobMonitor() {}


}


