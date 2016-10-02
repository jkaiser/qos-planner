//
// Created by Jürgen Kaiser on 01.10.16.
//

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
    std::string ost_limits_file;
    std::mutex scheduler_mut;

    std::map<std::string, float> osts_max_mbs_limits_;

    std::shared_ptr<ScheduleState> schedule;
    std::shared_ptr<JobMonitor> job_monitor;
    std::shared_ptr<Lustre> lustre;


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
