//
// Created by jkaiser on 17.03.16.
//

#ifndef QOS_PLANNER_JOBMONITOR_H
#define QOS_PLANNER_JOBMONITOR_H

#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <set>
#include "Job.h"
#include "ScheduleState.h"
#include "JobPriorityQueue.h"

namespace common {

/**
 * The JobMonitor tracks the time points when NRS settings for a given job should be set or unset. The Monitor
 * then performs the actions at the defined times.
 */
class JobMonitor {


private:
    bool monitor_thread_started;
    std::thread monitor_thread;
    bool monitor_thread_exit_flag = false;

    ScheduleState *scheduleState;

    std::mutex job_priority_queue_mutex;
    JobPriorityQueue job_priority_queue;

    /** in-flight jobs are jobs which currently are processed by the internal thread
     *  This distinction is necessary because UnregisterJob(...) might be called during
     *  the processing of a job event.
     */
    std::mutex in_flight_jobs_mutex;
    std::condition_variable in_flight_jobs_cv;
    std::set<std::string> in_flight_jobs;


    // function the monitor thread executes
    void Monitor();

    void Handle(const std::string &jobid, Job::JobEvent event);
    bool StartJob(const std::string &jobid);
    bool StopJob(const std::string &jobid);

public:
    JobMonitor(ScheduleState *st);
    bool Init();
    bool TearDown();
    bool RegisterJob(const Job &job);
    bool UnregisterJob(const Job &job);
};
}

#endif //QOS_PLANNER_JOBMONITOR_H
