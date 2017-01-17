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

#ifndef QOS_PLANNER_JOBSCHEDULERSTATICWORKLOADS_H
#define QOS_PLANNER_JOBSCHEDULERSTATICWORKLOADS_H

#include "JobScheduler.h"

#include <map>

#include <memory>
#include "ScheduleState.h"
#include "JobMonitor.h"
#include "Lustre.h"
#include "JobScheduler.h"

namespace common {

class JobSchedulerStaticWorkloads : public JobScheduler {

private:
    std::string ost_limits_file_;
    std::mutex scheduler_mut_;

    std::map<std::string, float> osts_max_mbs_limits_;

    std::shared_ptr<ScheduleState> schedule_;
    std::shared_ptr<JobMonitor> job_monitor_;
    std::shared_ptr<Lustre> lustre_;


    bool GetMaxLoadInTimeInterval(std::string ost,
                                  std::chrono::system_clock::time_point start,
                                  std::chrono::system_clock::time_point end,
                                  uint32_t *maxLoadMBSec);

    bool AreEnoughResAvail(const Job &job, const std::string &ost, uint32_t max_ost_mb_sec);

public:

    JobSchedulerStaticWorkloads(std::shared_ptr<ScheduleState> &schedule, std::shared_ptr<JobMonitor> job_monitor,
                                std::shared_ptr<Lustre> lustre, std::string &ost_limits_file);

    virtual bool Init() override;
    virtual bool Teardown() override;
    virtual bool ScheduleJob(Job &job) override;

    virtual bool RemoveJob(const std::string &jobid) override;

    void UpdateLimits(const std::map<std::string, uint32_t> &new_limits);

    bool JobDoesNotExist(const std::string &jobid, Job::JobState &state) const;
};

}

#endif //QOS_PLANNER_JOBSCHEDULERSTATICWORKLOADS_H
