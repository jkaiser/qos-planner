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
