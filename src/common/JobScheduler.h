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

#ifndef QOS_PLANNER_JOBSCHEDULER_H
#define QOS_PLANNER_JOBSCHEDULER_H

#include <string>
#include "Job.h"

namespace common {

class JobScheduler {
public:
    /**
     * Tries to schedule the given job with respect to the current schedule and the resource requirements of the job. The
     * call fails if the job
     *
     * Returns
     * true:    if the job was scheduled successfully
     * false:   else
     */
    virtual bool ScheduleJob(Job &job) = 0;

    /**
     * Removes the job from the schedule. If the job currently is active, the call also will remove any existing Lustre
     * settings for the job.
     *
     * Returns:
     * true:    if the job was removed successfully
     * false:   else
     */
    virtual bool RemoveJob(const std::string &jobid) = 0;

    virtual bool Init() = 0;
    virtual bool Teardown() = 0;
};
}

#endif //QOS_PLANNER_JOBSCHEDULER_H
