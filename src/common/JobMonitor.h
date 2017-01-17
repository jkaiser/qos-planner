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

#ifndef QOS_PLANNER_JOBMONITOR_H
#define QOS_PLANNER_JOBMONITOR_H

#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <set>
#include <memory>
#include <condition_variable>
#include "Job.h"
#include "MemoryScheduleState.h"
#include "JobPriorityQueue.h"
#include "Lustre.h"
#include "RuleManager.h"

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
    uint32_t waiting_time_sec_ = 5;

    bool monitor_thread_started_ = false;
    std::thread monitor_thread_;
    bool monitor_thread_exit_flag_ = false;
    bool monitor_thread_is_active_ = false;
    std::condition_variable monitor_thread_finish_cv_;

    std::shared_ptr<ScheduleState> scheduleState_;

    std::mutex job_priority_queue_mutex_;
    std::condition_variable job_priority_queue_cv_;
    JobPriorityQueue job_priority_queue_;

    /** in-flight jobs are jobs which currently are processed by the internal thread
     *  This distinction is necessary because UnregisterJob(...) might be called during
     *  the processing of a job event.
     */
    std::mutex in_flight_jobs_mutex_;
    std::condition_variable in_flight_jobs_cv_;
    std::set<std::string> in_flight_jobs_;

    std::shared_ptr<RuleManager> rule_manager_;

//    const std::string lustre_tbf_rule_postfix = "_qosp";
//    std::shared_ptr<Lustre> lustre;

    // function the monitor thread executes
    void Monitor();

    void Handle(const std::string &jobid, Job::JobEvent event);
    bool StartJob(const std::string &jobid);
    bool StopJob(const std::string &jobid);

    void FillWithExistingJobs();
    bool isThereAReadyJob() const;

    void RemoveJobFromPrioQueue(const Job &job);
    void BlockJob(const Job &job);
    void UnblockJob(const Job &job);
public:
    JobMonitor();
    JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<RuleManager> rule_manager);

    /**
     * waiting_time_sec: The time the internal threads sleeps before checking for an exit flag (set by a Teardown call)
     */
    JobMonitor(std::shared_ptr<ScheduleState> st, std::shared_ptr<RuleManager> rule_manager, uint32_t waiting_time_sec);

    virtual bool Init();

    virtual bool TearDown();

    virtual bool RegisterJob(const Job &job);

    virtual bool UnregisterJob(const Job &job);

    bool SetNrsRules(const std::string &basic_string);
};
}

#endif //QOS_PLANNER_JOBMONITOR_H
