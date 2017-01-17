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

#include "Server.h"

#include <unordered_set>

#include <spdlog/spdlog.h>

Server::Server(const std::string &ip_port, const std::string &root_path,
               std::shared_ptr<common::Planner> planner) : ip_port_(ip_port), context_(nullptr), root_path_(root_path), planner_(planner) {
}


bool Server::Init() {
    spdlog::get("console")->info("init server");

    if (!root_path_.empty()) {
        return false;   // classes with permanent data structures are not yet implemented
    }

    srandom ((unsigned) time (NULL));
    initZMQ();

    return true;
}

void Server::initZMQ() {
    context_ = new zmq::context_t (1);
    server_.reset(new zmq::socket_t(*context_, ZMQ_REP));
    spdlog::get("console")->info("will listen on: {}", ip_port_);
    server_->bind("tcp://" + ip_port_);
}

bool Server::TearDown() {
    // TODO: stop internal thread for accepting requests first
    return true;
}

void Server::Serve() {

    spdlog::get("console")->info("start serving");

    while (1) {
        std::string request = s_recv(*server_);

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

        ProcessMessage(msg);

        msg.release_request();
        msg.set_type(rpc::Message::REPLY);
        s_send (*server_, msg.SerializeAsString());
    }
}

bool Server::ProcessMessage(rpc::Message &msg) const {
    switch (msg.request().type()) {
            case rpc::Request::RESERVE:
                return planner_->ServeJobSubmission(msg);
            case rpc::Request::DELETE:
                return planner_->ServeJobRemove(msg);
            case rpc::Request::LISTJOBS:
                return planner_->ServeListJobs(msg);
            default :
                return false;
        }
}

void Server::ProcessUnparsableMsg(rpc::Message &msg) const {
    msg.set_type(rpc::Message::REPLY);
    msg.mutable_reply()->set_rc(1);
    s_send (*server_, msg.SerializeAsString());
}
