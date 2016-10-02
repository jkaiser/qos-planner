//
// Created by jkaiser on 29.03.16.
//

#include "Server.h"

#include <unordered_set>


#include <spdlog/spdlog.h>

Server::Server(const std::string &ip_port, const std::string &root_path,
               std::shared_ptr<common::Planner> planner) : ip_port_(ip_port), root_path(root_path), planner(planner) {
}


bool Server::Init() {
    spdlog::get("console")->info("Init server");

    if (!root_path.empty()) {
        return false;   // classes with permanent data structures are not yet implemented
    }

    srandom ((unsigned) time (NULL));

    initZMQ();

    return true;
}

void Server::initZMQ() {
    zmq::context_t context(1);
    server.reset(new zmq::socket_t(context, ZMQ_REP));
    server->bind(ip_port_);
}

bool Server::TearDown() {
    // TODO: stop internal thread for accepting requests first
    return true;
}

void Server::Serve() {

    spdlog::get("console")->info("start serving");

    while (1) {
        std::string request = s_recv(*server);

        std::cout << "I: normal request (" << request << ")" << std::endl;

        rpc::Message msg;
        if (!msg.ParseFromString(request)) {    // is it a valid parseable msg?
            ProcessUnparsableMsg(msg);
            continue;
        }

        if (msg.type() == rpc::Message::REPLY) {
           // invalid atm. We don't send requests and, therefore, don't expect replies.
            continue;
        }

        bool success;
        switch (msg.request().type()) {
            case rpc::Request::RESERVE:
                success = planner->ServeJobSubmission(msg.request().resourcerequest());
                break;
            case rpc::Request::DELETE:
                success = planner->ServeJobRemove(msg.request().deleterequest());
                break;
            case rpc::Request::LISTJOBS:
                success = planner->ServeListJobs(msg.request().listjobsrequest(),
                                                 std::shared_ptr<rpc::Reply>(msg.mutable_reply()));
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

        s_send (*server, msg.SerializeAsString());
    }
}

void Server::ProcessUnparsableMsg(rpc::Message &msg) const {
    msg.set_type(rpc::Message::REPLY);
    msg.mutable_reply()->set_rc(1);
    s_send (*server, msg.SerializeAsString());
}
