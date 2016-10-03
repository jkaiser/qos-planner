//
// Created by jkaiser on 5/23/16.
//

#include "Planner.h"
#include "JobSchedulerStaticWorkloads.h"
#include <unordered_set>
#include <regex>

#include <spdlog/spdlog.h>


namespace common {


Planner::Planner(std::string &root_path, std::string &ost_limits_file) : root_path(root_path), ost_limits_file(ost_limits_file) {
    lustre.reset(new LocalLustre());
    schedule.reset(new MemoryScheduleState());
    jobMonitor.reset(new JobMonitor(schedule, lustre, 10));
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


bool Planner::ServeJobSubmission(const rpc::Request_ResourceRequest &request) {
    if (request.files_size() == 0) {    // noop
        return true;
    }

    auto tstart = std::chrono::system_clock::now();
    auto tend = std::chrono::system_clock::from_time_t(request.tstop());

    int min_read_throughput = 0;
    std::unordered_set<std::string> osts_set;

    if (min_read_throughput < request.throughputmb()) {
        min_read_throughput = request.throughputmb();
    }

    for (auto it = request.files().begin(); it != request.files().end(); it++) {

        // get the osts for this file
        std::shared_ptr<std::vector<std::string>> osts(new std::vector<std::string>);
        if (!lustre->GetOstsForFile(*it, std::shared_ptr<std::vector<std::string>>(osts))) {
            return false;
        }

        for (auto ost : *osts) {
            osts_set.insert(ost);
        }
    }

    auto osts = std::vector<std::string>(osts_set.begin(), osts_set.end());
    common::Job job(request.id(), tstart, tend, min_read_throughput);
    job.setState(common::Job::JobState::INITIALIZED);
    job.setOsts(osts);

    return scheduler->ScheduleJob(job);
}


bool Planner::ServeJobRemove(const rpc::Request_DeleteRequest &msg) {
    for (int i = 0; i < msg.id_size(); i++) {
        if (!schedule->RemoveJob(msg.id(i))) {
            // TODO: add error logging
        }
    }
    return true;    // we guarantee that it is deleted -> always true
}



bool Planner::ServeListJobs(const rpc::Request_ListJobsRequest &msg, std::shared_ptr<rpc::Reply> reply_msg){

    if (msg.regex().empty()) {
        return false;
    }

    std::regex r(msg.regex(), std::regex::grep);


    auto jobs = schedule->GetAllJobs();

    std::string reply;

    for (auto& kv : *jobs) {
        // if k.first matches regex
        if (std::regex_search(kv.first, r)) {
            reply += kv.first + "\t" + Job::JobStateToString(kv.second->getState()) + "\n";
        }
    }

    if (reply.empty()) {
        reply = "no matches found\n";
        reply_msg->set_rc(-1);
    } else {
        reply_msg->set_rc(0);
    }

    reply_msg->set_return_msg(reply);

    return true;
}


}
