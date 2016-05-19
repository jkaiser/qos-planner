//
// Created by jkaiser on 29.03.16.
//

#include "Server.h"

#include <zhelpers.hpp>
#include <unordered_set>


Server::Server(const std::string &root_path) : root_path(root_path) {
    if (root_path.empty()) {
        lustre.reset(new common::LocalLustre());
        cluster_state.reset(new common::MemoryClusterState());
        schedule_state.reset(new common::MemoryScheduleState());
        job_monitor.reset(new common::JobMonitor(schedule_state, lustre));
        scheduler.reset(new common::Scheduler(schedule_state, job_monitor, cluster_state, lustre));
    }
    // else path not yet implemented as we've not yet implemented the classes using persistent data structures
}


bool Server::Init() {

    if (!root_path.empty()) {
        return false;   // classes with permanent data structures are not yet implemented
    }

    if (!schedule_state->Init()) {
        return false;
    }
    if (!cluster_state->Init()) {
        return false;
    }
    if (!job_monitor->Init()) {
        cluster_state->TearDown();
        return false;
    }
    if (!scheduler->Init()) {
        cluster_state->TearDown();
        job_monitor->TearDown();
        return false;
    }
    return true;
}

bool Server::TearDown() {

    bool return_val = true;
    // TODO: stop internal thread for accepting requests first

    return_val &= scheduler->TearDown();
    return_val &= job_monitor->TearDown();
    return_val &= cluster_state->TearDown();
    return_val &= schedule_state->TearDown();

    return return_val;
}

void Server::Serve(const std::string ip_port) {

    srandom ((unsigned) time (NULL));


    zmq::context_t context(1);
    zmq::socket_t server(context, ZMQ_REP);
    server.bind(ip_port);

    while (1) {
        std::string request = s_recv (server);

        std::cout << "I: normal request (" << request << ")" << std::endl;

        rpc::Message msg;
        bool success;
        switch (msg.type()) {
            case rpc::Message::REQUEST :
                success = ServeJobSubmission(msg.request().resourcerequest());
                break;
            default :
                success = false;
                break;
        }
        msg.release_request();
        msg.set_type(rpc::Message::REPLY);
        if (success) {
            msg.mutable_reply()->set_rc(0);
        } else {
            msg.mutable_reply()->set_rc(1);
        }

        s_send (server, msg.SerializeAsString());
    }
}


bool Server::ServeJobSubmission(const rpc::Request_ResourceRequest &request) {
    if (request.requestedresources().size() == 0 ) {    // noop
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

bool Server::ServeJobRemove(const rpc::Request_DeleteRequest &msg) {
    return false;
}










