//
// Created by jkaiser on 29.03.16.
//

#include "Server.h"

#include <zhelpers.hpp>
#include <unordered_set>


Server::Server(const std::string &root_path,
               std::shared_ptr<common::Planner> planner) : root_path(root_path), planner(planner) {
}


bool Server::Init() {

    if (!root_path.empty()) {
        return false;   // classes with permanent data structures are not yet implemented
    }
    return true;
}

bool Server::TearDown() {

    // TODO: stop internal thread for accepting requests first
    return planner->TearDown();
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
        if (!msg.ParseFromString(request)) {    // is it a valid parseable msg?
            msg.set_type(rpc::Message::REPLY);
            msg.mutable_reply()->set_rc(1);
            s_send (server, msg.SerializeAsString());
            continue;
        }

        if (msg.type() == rpc::Message::REPLY) {
           // invalid atm. We don't send requests and, therefore, don't expect repies.
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

        s_send (server, msg.SerializeAsString());
    }
}
