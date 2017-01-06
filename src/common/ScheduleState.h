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
    virtual const std::map<std::string, std::list<Job*>> *GetClusterState() = 0;

    /**
     * Returns the schedule of a single node.
     */
    virtual const std::list<Job*> *GetOSTState(const std::string &ost) = 0;

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
    virtual std::map<std::string, Job*> *GetAllJobs() = 0;
};

/**
 * Main memory based implementation of the ScheduleState.
 */
class MemoryScheduleState : public ScheduleState {

private:
    /**
     * Mappings from each ost to the sorted list of of jobs for that ost (ost -> [jobs] mappings).
     * TODO: This data structure mainly is used for checking for available resources in a given time interval. The
     * operation is O(n) for the std::list version. Switch to a skiplist if this turns out to be a bottleneck.
     */
    //
    std::map<std::string, std::list<Job*>> schedule_;

    std::map<std::string, Job*> jobs_;

public:

    virtual bool Init() override;
    virtual bool TearDown() override;
    virtual const std::map<std::string, std::list<Job*>> *GetClusterState() override;
    virtual const std::list<Job*> *GetOSTState(const std::string &ost) override;
    virtual void Reset() override;
    virtual bool AddJob(const std::string &jobid, const Job &job, const std::vector<std::string> &osts) override;
    virtual bool RemoveJob(const std::string &jobid) override;
    virtual bool UpdateJob(std::string jobid, Job::JobState new_state) override;
    virtual bool GetJobThroughput(const std::string jobid, uint32_t *throughput) override;
    virtual bool GetJobOstIds(const std::string &jobid, std::vector<std::string> &osts_out) override;
    virtual bool GetJobStatus(const std::string jobid, Job::JobState *state) override;
    virtual bool GetJobEnd(const std::string jobid, std::chrono::system_clock::time_point *tend) override;
    virtual std::map<std::string, Job*> *GetAllJobs() override;
};
}

#endif //QOS_PLANNER_SCHEDULESTATE_H
