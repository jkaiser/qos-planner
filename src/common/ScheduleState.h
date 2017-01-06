//
// Created by jkaiser on 15.03.16.
//

#ifndef QOS_PLANNER_SCHEDULESTATE_H
#define QOS_PLANNER_SCHEDULESTATE_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <mutex>

#include "Job.h"

namespace common {

/**
 * Abstract class. The ScheduleState holds and manages the schedule. The schedule is the set of accepted jobs.
 */
class ScheduleState {

protected:
    std::mutex schedule_mut_;
public:

    virtual bool Init() = 0;

    virtual bool TearDown() = 0;

    /**
     * Returns the schedules of the full cluster.
     */
    virtual const std::map<std::string, std::list<Job *>> *GetClusterState() = 0;

    /**
     * Returns the schedule of a single node.
     */
    virtual const std::list<Job *> *GetOSTState(const std::string &ost) = 0;

    virtual void Reset() = 0;

    /**
     * Tries to add a job to the given OSTs.
     */
    virtual bool AddJob(const std::string &jobid, const Job &job, const std::vector<std::string> &osts) = 0;

    virtual bool RemoveJob(const std::string &jobid) = 0;

    virtual bool UpdateJob(std::string jobid, Job::JobState new_state) = 0;

    virtual bool GetJobThroughput(const std::string jobid, uint32_t *throughput) = 0;

    virtual bool GetJobOstIds(const std::string &jobid, std::vector<std::string> &osts_out) = 0;

    virtual bool GetJobStatus(const std::string jobid, Job::JobState *state) = 0;

    virtual bool GetJobEnd(const std::string jobid, std::chrono::system_clock::time_point *tend) = 0;

    /**
     * Returns a (deep) copy of all current jobs. NOTE: This is an expensive operation. Use it with care!
     */
    virtual std::map<std::string, Job *> *GetAllJobs() = 0;
};
}

#endif //QOS_PLANNER_SCHEDULESTATE_H
