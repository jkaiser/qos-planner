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
    const std::string root_path_;
    const std::string ost_limits_file_;
    std::shared_ptr<Lustre> lustre_;
    std::shared_ptr<JobScheduler> scheduler_;
    std::shared_ptr<ScheduleState> schedule_;
    std::shared_ptr<RuleManager> rule_manager_;
    std::shared_ptr<JobMonitor> job_monitor_;

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
