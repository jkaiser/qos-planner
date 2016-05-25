//
// Created by jkaiser on 5/23/16.
//

#ifndef QOS_PLANNER_PLANNER_H
#define QOS_PLANNER_PLANNER_H

#include <string>
#include <memory>
#include "Lustre.h"
#include "Scheduler.h"
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
    Planner(std::string root_path);
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
    std::shared_ptr<Lustre> lustre;
    std::shared_ptr<ClusterState> cluster;
    std::shared_ptr<Scheduler> scheduler;
    std::shared_ptr<ScheduleState> schedule;
    std::shared_ptr<JobMonitor> jobMonitor;

};

}

#endif //QOS_PLANNER_PLANNER_H
