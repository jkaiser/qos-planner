//
// Created by jkaiser on 22.03.16.
//

#ifndef QOS_PLANNER_SCHEDULER_H
#define QOS_PLANNER_SCHEDULER_H

#include <memory>
#include "ScheduleState.h"
#include "JobMonitor.h"
#include "ClusterState.h"
#include "Lustre.h"

namespace common {

/**
 *
 * Example:
 * Scheduler sched(schedule, job_monitor, cluster_state);
 *
 * auto j = new Job(...)
 *
 * sched.ScheduleJob(j);
 * sched.Remove(j.getJobid());
 *
 */
class Scheduler {

private:
    std::mutex scheduler_mut;

    std::shared_ptr<ClusterState> cluster_state;
    std::shared_ptr<ScheduleState> schedule;
    std::shared_ptr<JobMonitor> job_monitor;
    std::shared_ptr<Lustre> lustre;


    bool GetMaxLoadInTimeInterval(std::string ost,
                                  std::chrono::system_clock::time_point start,
                                  std::chrono::system_clock::time_point end,
                                  uint32_t *maxLoadMBSec);

    bool AreEnoughResAvail(const Job &job, uint32_t max_ost_mb_sec, const OSTWorkload &node_state) const;

public:
    /**
     * Constructor. It is assumed that the given instances already are initialized and ready to be used.
     */
    Scheduler(std::shared_ptr<ScheduleState> &schedule, std::shared_ptr<JobMonitor> job_monitor,
              std::shared_ptr<ClusterState> cluster_state, std::shared_ptr<Lustre> lustre);

    bool Init();
    bool TearDown();
    /**
     * Tries to schedule the given job with respect to the current schedule and the resource requirements of the job. The
     * call fails if the job
     *
     * Returns
     * true:    if the job was scheduled successfully
     * false:   else
     */
    bool ScheduleJob(Job &job);

    /**
     * Removes the job from the schedule. If the job currently is active, the call also will remove any existing Lustre
     * settings for the job.
     *
     * Returns:
     * true:    if the job was removed successfully
     * false:   else
     */
    bool RemoveJob(const std::string &jobid);


    bool JobDoesNotExist(const std::string &jobid, Job::JobState &state) const;
};

}

#endif //QOS_PLANNER_SCHEDULER_H
