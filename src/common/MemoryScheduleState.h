//
// Created by Jürgen Kaiser on 06.01.17.
//

#ifndef QOS_PLANNER_MEMORYSCHEDULESTATE_H
#define QOS_PLANNER_MEMORYSCHEDULESTATE_H

#include "ScheduleState.h"

namespace common {

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

#endif //QOS_PLANNER_MEMORYSCHEDULESTATE_H
