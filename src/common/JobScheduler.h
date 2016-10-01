//
// Created by Jürgen Kaiser on 01.10.16.
//

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
};
}

#endif //QOS_PLANNER_JOBSCHEDULER_H
