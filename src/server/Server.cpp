//
// Created by jkaiser on 29.03.16.
//

#include "Server.h"

#include <unordered_set>

#include <spdlog/spdlog.h>

Server::Server(const std::string &ip_port, const std::string &root_path,
               std::shared_ptr<common::Planner> planner) : ip_port_(ip_port), context(nullptr), root_path(root_path), planner(planner) {
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
    context = new zmq::context_t (1);
    server.reset(new zmq::socket_t(*context, ZMQ_REP));
    spdlog::get("console")->info("will listen on: {}", ip_port_);
    server->bind("tcp://" + ip_port_);
}

bool Server::TearDown() {
    // TODO: stop internal thread for accepting requests first
    return true;
}

void Server::Serve() {

    spdlog::get("console")->info("start serving");

    while (1) {
        std::string request = s_recv(*server);

        rpc::Message msg;
        if (!msg.ParseFromString(request)) {    // is it a valid parseable msg?
            spdlog::get("console")->error("got unparsable message");
            ProcessUnparsableMsg(msg);
            continue;
        }

        spdlog::get("console")->debug("got message {}", msg.DebugString());

        if (msg.type() == rpc::Message::REPLY) {
            spdlog::get("console")->error("got an invalid reply");
           // invalid atm. We don't send requests and, therefore, don't expect replies.
            continue;
        }

        bool success;
        success = ProcessMessage(msg);

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

bool Server::ProcessMessage(rpc::Message &msg) const {
    switch (msg.request().type()) {
            case rpc::Request::RESERVE:
                return planner->ServeJobSubmission(msg.request().resourcerequest());
            case rpc::Request::DELETE:
                return planner->ServeJobRemove(msg.request().deleterequest());
            case rpc::Request::LISTJOBS:
                return planner->ServeListJobs(msg.request().listjobsrequest(),
                                                 std::shared_ptr<rpc::Reply>(msg.mutable_reply()));
            default :
                return false;
        }
}

void Server::ProcessUnparsableMsg(rpc::Message &msg) const {
    msg.set_type(rpc::Message::REPLY);
    msg.mutable_reply()->set_rc(1);
    s_send (*server, msg.SerializeAsString());
}
