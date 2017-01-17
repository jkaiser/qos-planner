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
    std::mutex scheduler_mut_;

    std::shared_ptr<ClusterState> cluster_state_;
    std::shared_ptr<ScheduleState> schedule_;
    std::shared_ptr<JobMonitor> job_monitor_;
    std::shared_ptr<Lustre> lustre_;


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
