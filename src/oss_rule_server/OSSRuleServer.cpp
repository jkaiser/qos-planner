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

#include "OSSRuleServer.h"

#include <util.h>
#include <spdlog/spdlog.h>

namespace oss_rule_server {

void OSSRuleServer::initZMQ() {
    context_ = new zmq::context_t (1);
    server_.reset(new zmq::socket_t(*context_, ZMQ_REP));
    spdlog::get("console")->info("will listen on: {}", ip_port_);
    server_->bind("tcp://" + ip_port_);
}

OSSRuleServer::OSSRuleServer(const std::string &ip_port) : ip_port_(ip_port), context_(nullptr){}

bool OSSRuleServer::Init() {
    spdlog::get("console")->info("init server");

    srandom ((unsigned) time (NULL));
    initZMQ();

    return true;
}

bool OSSRuleServer::TearDown() {
    // TODO: stop internal thread for accepting requests first
    return true;
}

void OSSRuleServer::Serve() {

    spdlog::get("console")->info("start serving");

    while (1) {
        std::string request = s_recv(*server_);

        rpc::OSSCommandMessage msg;
        if (!msg.ParseFromString(request)) {    // is it a valid parseable msg?
            spdlog::get("console")->error("got unparsable message");
            ProcessUnparseableMsg(msg);
            continue;
        }

        spdlog::get("console")->debug("got message {}", msg.DebugString());

        if (msg.type() == rpc::OSSCommandMessage::REPLY) {
            spdlog::get("console")->error("got an invalid reply");
            // invalid atm. We don't send requests and, therefore, don't expect replies.
            continue;
        }

        ProcessMessage(msg);

        msg.release_request();
        msg.set_type(rpc::OSSCommandMessage::REPLY);
        s_send (*server_, msg.SerializeAsString());
    }
}

void OSSRuleServer::ProcessUnparseableMsg(rpc::OSSCommandMessage &msg) const {
    msg.set_type(rpc::OSSCommandMessage::REPLY);
    msg.mutable_reply()->set_rc(1);
    msg.mutable_reply()->set_return_msg("unparseable message");
    s_send (*server_, msg.SerializeAsString());
}

bool OSSRuleServer::ProcessMessage(rpc::OSSCommandMessage &msg) const {
    switch (msg.type()) {
        case rpc::OSSCommandMessage::REQUEST:
            return ServeCommand(msg);
        case rpc::OSSCommandMessage::REPLY:
            return ServeReply(msg);

        default:
            return false;
    }
}

bool OSSRuleServer::ServeCommand(rpc::OSSCommandMessage &message) const {

    auto out = std::make_shared<std::string>();
    if (!common::exec(message.request().command().c_str(), out)) {
        message.clear_request();
        message.set_type(rpc::OSSCommandMessage::REPLY);
        message.mutable_reply()->set_return_msg(*out);
        message.mutable_reply()->set_rc(-1);
    } else {
        message.clear_request();
        message.mutable_reply()->set_rc(0);
    }

    return true;
}

bool OSSRuleServer::ServeReply(rpc::OSSCommandMessage &message) const {
    message.clear_request();
    message.mutable_reply()->set_return_msg("invalid message type: Reply");
    message.mutable_reply()->set_rc(-1);
    return true;
}

}