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

#include "Planner.h"
#include "JobSchedulerStaticWorkloads.h"
#include "OstIdsConverter.h"
#include "OstIpsCache.h"
#include "SSHRuleSetter.h"
#include "ListJobsFormatter.h"

#include <spdlog/spdlog.h>

namespace common {


Planner::Planner(std::string &root_path, std::string &ost_limits_file) : root_path_(root_path),
                                                                         ost_limits_file_(ost_limits_file) {
    lustre_ = std::make_shared<LocalLustre>();
    schedule_ = std::make_shared<MemoryScheduleState>();

    auto ocache = std::make_shared<OstIpsCache>(lustre_);
    auto id_converter = std::make_shared<OstIdsConverter>(lustre_);
    auto rule_setter = std::make_shared<SSHRuleSetter>();
    rule_manager_ = std::make_shared<RuleManager>(rule_setter, ocache, id_converter);

    job_monitor_ = std::make_shared<JobMonitor>(schedule_, rule_manager_, 5);

    scheduler_ = std::make_shared<JobSchedulerStaticWorkloads>(schedule_, job_monitor_, lustre_, ost_limits_file);
}

bool Planner::Init() {

    if (!lustre_->Init()) {
        spdlog::get("console")->critical("Initializing lustre connector failed");
        return false;
    }

    if (!schedule_->Init()) {
        spdlog::get("console")->critical("Initializing schedule failed");
        return false;
    }

    if (!job_monitor_->Init()) {
        schedule_->TearDown();
        spdlog::get("console")->critical("Initializing job monitor failed");
        return false;
    }

    if (!scheduler_->Init()) {
        spdlog::get("console")->critical("Initializing scheduler failed");
        job_monitor_->TearDown();
        schedule_->TearDown();
        return false;
    }

    return true;
}

bool Planner::TearDown() {
    bool ret = true;

    ret &= job_monitor_->TearDown();
    job_monitor_.reset();

    ret &= schedule_->TearDown();
    schedule_.reset();

    return ret;
}


bool Planner::ServeJobSubmission(rpc::Message &msg) {
    const rpc::Request_ResourceRequest &request = msg.request().resourcerequest();
    if (request.files_size() == 0) {    // noop
        return true;
    }

    std::unordered_set<std::string> osts_set;
    if (!tryComputeOstSetOfRequest(request, osts_set)) {
        spdlog::get("console")->error("couldn't get list of OSTs for reservation");
        return false;
    }

    auto tstart = std::chrono::system_clock::now();
    auto tend =  std::chrono::time_point<std::chrono::system_clock>::max();
    if (request.durationsec() > 0) {
        tend = tstart + std::chrono::seconds(request.durationsec());
    }

    auto job = BuildJob(request, tstart, tend, osts_set);
    bool success = scheduler_->ScheduleJob(*job);
    msg.mutable_reply()->set_rc((success) ? 0 : -1);
    return success;
}

std::shared_ptr<Job> Planner::BuildJob(const rpc::Request_ResourceRequest &request,
                                       const std::chrono::time_point<std::chrono::system_clock> &tstart,
                                       const std::chrono::time_point<std::chrono::system_clock> &tend,
                                       std::unordered_set<std::string> &osts_set) const {
    std::shared_ptr<Job> job(new Job(request.id(), tstart, tend, request.throughputmb()));
    job->setState(Job::INITIALIZED);
    auto osts = std::vector<std::string>(osts_set.begin(), osts_set.end());
    job->setOsts(osts);
    return job;
}

bool Planner::tryComputeOstSetOfRequest(const rpc::Request_ResourceRequest &request,
                                        std::unordered_set<std::string> &osts_set) const {
    for (auto it = request.files().begin(); it != request.files().end(); it++) {
        std::shared_ptr<std::vector<std::string>> osts(new std::vector<std::string>());
        if (!lustre_->GetOstsForFile(*it, osts)) {
            return false;
        }

        for (auto ost : *osts) {
            osts_set.insert(ost);
        }
    }
    return true;
}


bool Planner::ServeJobRemove(rpc::Message &msg) {
    const rpc::Request_DeleteRequest &request = msg.request().deleterequest();

    bool success = true;
    for (int i = 0; i < request.id_size(); i++) {

        job_monitor_->UnregisterJob(request.id(i));

        if (!schedule_->RemoveJob(request.id(i))) {
//            spdlog::get("console")->warn("removing of job {} failed", request.id(i));
//            success = false;
        }
    }

    msg.mutable_reply()->set_rc((success) ? 0 : 1);
    return success;    // we guarantee that it is deleted -> always true
}


bool Planner::ServeListJobs(rpc::Message &msg) {
    AddDefaultRegexIfEmpty(msg);

    const rpc::Request_ListJobsRequest &request = msg.request().listjobsrequest();
    auto jobs = schedule_->GetAllJobs();
    std::vector<Job *> job_list = FilterJobs(request, jobs);

    std::shared_ptr<rpc::Reply> reply_msg(new (rpc::Reply));
    AddJobsToReply(reply_msg, jobs, job_list);

    reply_msg->set_rc(0);
    msg.mutable_reply()->CopyFrom(*reply_msg);
    return true;
}

std::vector<Job *>
Planner::FilterJobs(const rpc::Request_ListJobsRequest &request, const std::map<std::string, Job *> *jobs) const {
    std::regex r(request.regex(), std::regex::grep);
    std::vector<Job *> job_list;
    job_list.reserve(jobs->size());
    for (auto &kv : *jobs) {
        if (regex_search(kv.first, r)) {
            job_list.push_back(kv.second);
        }
    }
    return job_list;
}

void Planner::AddDefaultRegexIfEmpty(rpc::Message &msg) const {
    if (msg.request().listjobsrequest().regex().empty()) {
        msg.mutable_request()->mutable_listjobsrequest()->set_regex(".*");
    }
}

void Planner::AddJobsToReply(std::shared_ptr<rpc::Reply> &reply_msg, const std::map<std::string, Job *> *jobs,
                             std::vector<Job *> &job_list) const {
    if (jobs->size() > 0) {
        ListJobsFormatter formatter;
        reply_msg->set_return_msg(*formatter.Format(job_list));
    } else {
        reply_msg->set_return_msg("no matches found\n");
    }
}


}
