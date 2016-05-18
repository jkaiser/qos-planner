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
#include "Lustre.h"

namespace common {

// TODO: this is for the long run: when a job is removed, some resources are freed on the OST. These won't be used
//       until the remaining NRS settings are adjusted (since the other ones limit the throughput of
//        the running applications)
/**
 * The JobMonitor tracks the time points when NRS settings for a given job should be set or unset. The Monitor
 * then performs the actions at the defined times.
 */
class JobMonitor {


private:
    // The internal thread sleeps for this amount of seconds before checking for a Teardown call
    uint32_t waiting_time_sec = 5;

    bool monitor_thread_started = false;
    std::thread monitor_thread;
    bool monitor_thread_exit_flag = false;

    std::shared_ptr<ScheduleState> scheduleState;
//    ScheduleState *scheduleState = nullptr;

    std::mutex job_priority_queue_mutex;
    JobPriorityQueue job_priority_queue;

    /** in-flight jobs are jobs which currently are processed by the internal thread
     *  This distinction is necessary because UnregisterJob(...) might be called during
     *  the processing of a job event.
     */
    std::mutex in_flight_jobs_mutex;
    std::condition_variable in_flight_jobs_cv;
    std::set<std::string> in_flight_jobs;

    const std::string lustre_tbf_rule_postfix = "_qos_sched_io_rule";
    std::shared_ptr<Lustre> lustre;

    // function the monitor thread executes
    void Monitor();

    void Handle(const std::string &jobid, Job::JobEvent event);
    bool StartJob(const std::string &jobid);
    bool StopJob(const std::string &jobid);

public:
    JobMonitor();
//    JobMonitor(common::ScheduleState *st, Lustre *lustre);
//    JobMonitor(common::ScheduleState *st, Lustre *lustre, uint32_t waiting_time_sec);
    JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<Lustre> lustre);
    /**
     * waiting_time_sec: The time the internal threads sleeps before checking for an exit flag (set by a Teardown call)
     */
    JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<Lustre> lustre, uint32_t waiting_time_sec);
    virtual bool Init();
    virtual bool TearDown();
    virtual bool RegisterJob(const Job &job);
    virtual bool UnregisterJob(const Job &job);
};
}

#endif //QOS_PLANNER_JOBMONITOR_H
