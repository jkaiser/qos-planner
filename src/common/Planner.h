//
// Created by jkaiser on 5/23/16.
//

#ifndef QOS_PLANNER_PLANNER_H
#define QOS_PLANNER_PLANNER_H

#include <string>
#include <memory>
#include <regex>
#include <unordered_set>
#include "Lustre.h"
#include "JobScheduler.h"
#include "JobMonitor.h"
#include "ScheduleState.h"
#include "ClusterState.h"
#include "RuleManager.h"
#include "../common/rpc/proto/message.pb.h"

namespace common {

/**
 * Planner p("/foo/bar");
 * p.Init();
 * p.ServeJobSubmission(...);
 * [...]
 * p.TearDown();
 */
class Planner {

public:
    Planner(std::string &root_path, std::string &ost_limits_file);
    bool Init();
    bool TearDown();

    /**
     * Processes the given resource request.
     */
    bool ServeJobSubmission(rpc::Message &msg);
    bool ServeJobRemove(rpc::Message &msg);
    bool ServeListJobs(rpc::Message &msg);

private:
    const std::string root_path;
    const std::string ost_limits_file;
    std::shared_ptr<Lustre> lustre;
    std::shared_ptr<JobScheduler> scheduler;
    std::shared_ptr<ScheduleState> schedule;
    std::shared_ptr<RuleManager> rule_manager;
    std::shared_ptr<JobMonitor> jobMonitor;

    void AddJobsToReply(std::shared_ptr<rpc::Reply> &reply_msg, const std::map<std::string, Job *> *jobs,
                        std::vector<Job *> &job_list) const;

    bool tryComputeOstSetOfRequest(const rpc::Request_ResourceRequest &request, std::unordered_set<std::string> &osts_set) const;

    std::shared_ptr<Job> BuildJob(const rpc::Request_ResourceRequest &request,
                                  const std::chrono::time_point<std::chrono::system_clock> &tstart,
                                  const std::chrono::time_point<std::chrono::system_clock> &tend,
                                  std::unordered_set<std::string> &osts_set) const;

    void AddDefaultRegexIfEmpty(rpc::Message &msg) const;

    std::vector<Job *> FilterJobs(const rpc::Request_ListJobsRequest &request, const std::map<std::string, Job *> *jobs) const;
};

}

#endif //QOS_PLANNER_PLANNER_H
