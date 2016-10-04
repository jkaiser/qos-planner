//
// Created by jkaiser on 5/23/16.
//

#ifndef QOS_PLANNER_PLANNER_H
#define QOS_PLANNER_PLANNER_H

#include <string>
#include <memory>
#include <regex>
#include "Lustre.h"
#include "JobScheduler.h"
#include "JobMonitor.h"
#include "ScheduleState.h"
#include "ClusterState.h"
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
    bool ServeJobSubmission(const rpc::Request_ResourceRequest &msg);
    bool ServeJobRemove(const rpc::Request_DeleteRequest &msg);
    bool ServeListJobs(const rpc::Request_ListJobsRequest &msg, std::shared_ptr<rpc::Reply> reply_msg);

private:
    const std::string root_path;
    const std::string ost_limits_file;
    std::shared_ptr<Lustre> lustre;
    std::shared_ptr<JobScheduler> scheduler;
    std::shared_ptr<ScheduleState> schedule;
    std::shared_ptr<JobMonitor> jobMonitor;

    std::vector<Job *> FilterJobs(const std::__1::basic_regex<char, std::__1::regex_traits<char>> &r,
                                  const std::map<std::string, Job *> *jobs) const;

    void AddJobsToReply(std::shared_ptr<rpc::Reply> &reply_msg, const std::map<std::string, Job *> *jobs,
                        std::vector<Job *> &job_list) const;
};

}

#endif //QOS_PLANNER_PLANNER_H
