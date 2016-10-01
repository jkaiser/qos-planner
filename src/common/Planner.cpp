//
// Created by jkaiser on 5/23/16.
//

#include "Planner.h"

#include <unordered_set>
#include <regex>

namespace common {


Planner::Planner(std::string root_path) : root_path(root_path) {

//    lustre = dynamic_pointer_cast(new LocalLustre());
    lustre = std::shared_ptr<LocalLustre>(new LocalLustre());
    cluster = std::shared_ptr<MemoryClusterState>(new MemoryClusterState(lustre));
    schedule = std::shared_ptr<MemoryScheduleState>(new MemoryScheduleState());
    jobMonitor = std::shared_ptr<JobMonitor>(new JobMonitor());
    scheduler = std::shared_ptr<JobSchedulerDynWorkloads>(new JobSchedulerDynWorkloads(schedule, jobMonitor, cluster, lustre));
}

bool Planner::Init() {

    //TODO: Add error logging
    if (!lustre->Init()) {
        return false;
    }

    if (!schedule->Init()) {
        return false;
    }

    if (!cluster->Init()) {
        schedule->TearDown();
        return false;
    }

    if (!jobMonitor->Init()) {
        cluster->TearDown();
        schedule->TearDown();
        return false;
    }

    if (!scheduler->Init()) {
        jobMonitor->TearDown();
        cluster->TearDown();
        schedule->TearDown();
        return false;
    }

    return true;
}

bool Planner::TearDown() {
    bool ret = true;

    //TODO: implement error messages
    ret &= scheduler->TearDown();
    scheduler.reset();

    ret &= jobMonitor->TearDown();
    jobMonitor.reset();

    ret &= cluster->TearDown();
    cluster.reset();

    ret &= schedule->TearDown();
    schedule.reset();

    return ret;
}


bool Planner::ServeJobSubmission(const rpc::Request_ResourceRequest &request) {
    if (request.requestedresources().size() == 0) {    // noop
        return true;
    }

    auto tstart = std::chrono::system_clock::from_time_t(request.requestedresources(0).tstart());
    auto tend = std::chrono::system_clock::from_time_t(request.requestedresources(0).tstop());

    int min_read_throughput = 0;
    std::unordered_set<std::string> osts_set;

    for (auto it = request.requestedresources().begin(); it != request.requestedresources().begin(); it++) {

        // update min_read_throughput
        if (min_read_throughput < it->throughputmb()) {
            min_read_throughput = it->throughputmb();
        }

        // get the osts for this file
        std::vector<std::string> osts;
        if (!lustre->GetOstsForFile(it->file(), std::shared_ptr<std::vector<std::string>>(&osts))) {
            return false;
        }

        for (auto ost : osts) {
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
