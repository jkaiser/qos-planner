//
// Created by jkaiser on 17.03.16.
//

#include "JobMonitor.h"

namespace common {

JobMonitor::JobMonitor(common::ScheduleState *st) {
    this->scheduleState = st;
}

bool JobMonitor::RegisterJob(const common::Job &job) {
    JobPriorityQueue::WaitingItem *wt = new JobPriorityQueue::WaitingItem(job.getJobid(), job.getTstart(), Job::JobEvent::JOBSTART);
    job_priority_queue.Push(wt);
    return true;
}

void JobMonitor::Handle(const std::string &jobid, Job::JobEvent event) {

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
}

bool JobMonitor::Init() {

    if (not monitor_thread_started) {
        monitor_thread = std::thread(&JobMonitor::Monitor, this);
        monitor_thread_started = true;
    } else { // something's wrong. Was it initialized before?
        return false;
    }


    return true;
}

void JobMonitor::Monitor() {

    while (!monitor_thread_exit_flag) {

        job_priority_queue_mutex.lock();
        if ((job_priority_queue.Peek() != nullptr ) &&
                    (job_priority_queue.Peek()->time_of_event < std::chrono::system_clock::now())) {
            JobPriorityQueue::WaitingItem *wt = job_priority_queue.Pop();
            job_priority_queue_mutex.unlock();

            Handle(wt->jobid, wt->eventType);

            delete wt;
        } else {
            job_priority_queue_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

    }
}

bool JobMonitor::TearDown() {
    if (!monitor_thread_exit_flag) {
        monitor_thread_exit_flag = true;
        std::chrono::seconds max_wait_time(30); // 30 seconds to wait
        auto tstart = std::chrono::system_clock::now();

        bool successful_joined = false;
        while (std::chrono::system_clock::now() < tstart + max_wait_time) {
            if (monitor_thread.joinable()) {
                monitor_thread.join();
                successful_joined = true;
                break;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (not successful_joined) {
            return false;
        }
    }

    return true;
}

bool JobMonitor::StartJob(const std::string &jobid) {
 // assuming start:

    bool ret_val = true;

    // 1) set the job to "in-flght"  -> internal data structure for that
    in_flight_jobs_mutex.lock();
    in_flight_jobs.insert(jobid);
    in_flight_jobs_mutex.unlock();

    // 2) set the NRS settings
    // TODO: set NRS settings


    // 3) update job status to ACTIVE
    if (ret_val) {
        ret_val &= scheduleState->UpdateJob(jobid, Job::ACTIVE);
    }

    // 4) remove in-flight state
    in_flight_jobs_mutex.lock();
    in_flight_jobs.erase(jobid);
    in_flight_jobs_mutex.unlock();

    return ret_val;
}

bool JobMonitor::StopJob(const std::string &jobid) {
    bool ret_val = true;

    // 1) set the job to "in-flght"
    // wait if the job is processed by other calls atm
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex);
    while (in_flight_jobs.find(jobid) != in_flight_jobs.end()) {
        in_flight_jobs_cv.wait(lock);
    }
    in_flight_jobs.insert(jobid);
    lock.unlock();


    // 2) set the NRS settings
    // TODO: set NRS settings


    // 3) update job status to DONE
    if (ret_val) {
        ret_val &= scheduleState->UpdateJob(jobid, Job::DONE);
    }

    // 4) remove in-flight state
    lock.lock();
    in_flight_jobs.erase(jobid);
    in_flight_jobs_cv.notify_all();
    lock.unlock();

    return ret_val;
}

bool JobMonitor::UnregisterJob(const Job &job) {

    bool ret_val = true;

    // check if job is in-flight and wait if so
    std::unique_lock<std::mutex> lock(in_flight_jobs_mutex);
    while (in_flight_jobs.find(job.getJobid()) != in_flight_jobs.end()) {
        in_flight_jobs_cv.wait(lock);
    }
    in_flight_jobs.insert(job.getJobid());
    lock.unlock();

    // stop the job
    ret_val = StopJob(job.getJobid());

    // remove the in-flight status
    lock.lock();
    in_flight_jobs.erase(job.getJobid());
    in_flight_jobs_cv.notify_all();
    lock.unlock();

    return ret_val;
}

}


