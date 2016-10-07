//
// Created by jkaiser on 5/23/16.
//

#include "Planner.h"
#include "JobSchedulerStaticWorkloads.h"

#include <spdlog/spdlog.h>
#include "ListJobsFormatter.h"

namespace common {


Planner::Planner(std::string &root_path, std::string &ost_limits_file) : root_path(root_path),
                                                                         ost_limits_file(ost_limits_file) {
    lustre.reset(new LocalLustre());
    schedule.reset(new MemoryScheduleState());
    jobMonitor.reset(new JobMonitor(schedule, lustre, 5));
    scheduler.reset(new JobSchedulerStaticWorkloads(schedule, jobMonitor, lustre, ost_limits_file));
}

bool Planner::Init() {

    if (!lustre->Init()) {
        spdlog::get("console")->critical("Initializing lustre connector failed");
        return false;
    }

    if (!schedule->Init()) {
        spdlog::get("console")->critical("Initializing schedule failed");
        return false;
    }

    if (!jobMonitor->Init()) {
        schedule->TearDown();
        spdlog::get("console")->critical("Initializing job monitor failed");
        return false;
    }

    if (!scheduler->Init()) {
        spdlog::get("console")->critical("Initializing scheduler failed");
        jobMonitor->TearDown();
        schedule->TearDown();
        return false;
    }

    return true;
}

bool Planner::TearDown() {
    bool ret = true;

    ret &= jobMonitor->TearDown();
    jobMonitor.reset();

    ret &= schedule->TearDown();
    schedule.reset();

    return ret;
}


bool Planner::ServeJobSubmission(rpc::Message &msg) {
    const rpc::Request_ResourceRequest &request = msg.request().resourcerequest();
    if (request.files_size() == 0) {    // noop
        return true;
    }

    std::unordered_set<std::string> osts_set;
    if (!tryComputeOstSetOfRequest(request, osts_set)) {
        return false;
    }

    auto tstart = std::chrono::system_clock::now();
    auto tend =  std::chrono::time_point<std::chrono::system_clock>::max();
    if (request.durationsec() > 0) {
        tend = tstart + std::chrono::seconds(request.durationsec());
    }

    auto job = BuildJob(request, tstart, tend, osts_set);
    bool success = scheduler->ScheduleJob(*job);
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
        if (!lustre->GetOstsForFile(*it, osts)) {
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
        if (!schedule->RemoveJob(request.id(i))) {
            spdlog::get("console")->warn("removing of job {} failed", request.id(i));
            success = false;
        }
    }

    msg.mutable_reply()->set_rc((success) ? 0 : 1);
    return success;    // we guarantee that it is deleted -> always true
}


bool Planner::ServeListJobs(rpc::Message &msg) {
    AddDefaultRegexIfEmpty(msg);

    const rpc::Request_ListJobsRequest &request = msg.request().listjobsrequest();
    auto jobs = schedule->GetAllJobs();
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
