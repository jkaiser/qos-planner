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
#include "JobScheduler.h"

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
class JobSchedulerDynWorkloads : public JobScheduler {

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

    bool JobDoesNotExist(const std::string &jobid, Job::JobState &state) const;

public:

    /**
     * Constructor. It is assumed that the given instances already are initialized and ready to be used.
     */
    JobSchedulerDynWorkloads(std::shared_ptr<ScheduleState> &schedule, std::shared_ptr<JobMonitor> job_monitor,
              std::shared_ptr<ClusterState> cluster_state, std::shared_ptr<Lustre> lustre);
    virtual bool Init() override;
    virtual bool Teardown() override;

    virtual bool ScheduleJob(Job &job) override;
    virtual bool RemoveJob(const std::string &jobid) override;
};

}

#endif //QOS_PLANNER_SCHEDULER_H
