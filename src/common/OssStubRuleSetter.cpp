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

#include "OssStubRuleSetter.h"

#include <spdlog/spdlog.h>

namespace common {

OssStubRuleSetter::OssStubRuleSetter() {
    context_ = new zmq::context_t(1);
}

OssStubRuleSetter::~OssStubRuleSetter() {
    context_->close();
}

bool OssStubRuleSetter::SetRule(const std::string &ip, const std::string &job_id, const std::string &rule_name,
                                uint32_t rpc_rate_limit) {
    std::string cmd = BuildLocalSetRuleCommand(job_id, rule_name, rpc_rate_limit);
    return RemotelyExecuteCmd(ip, cmd);
}

bool OssStubRuleSetter::RemoveRule(const std::string &ip, const std::string &rule_name, const std::string &job_id) {
    std::string cmd = BuildLocalRemoveRuleCommand(job_id, rule_name);
    return RemotelyExecuteCmd(ip, cmd);
}

bool OssStubRuleSetter::RemotelyExecuteCmd(const std::string &ip, const std::string &cmd) {
    auto msg = BuildCommandMessage(cmd);

    if (ip.size() == 0) {
        return false;
    }
    ipPort_ = ip + ":" + kDefaultPort;

    InitializeZmqSocket();

    std::string reply;
    if (!TrySendRequestAndReceiveReply(msg, reply)) {
        return false;
    }

    return ProcessReply(reply);
}

void OssStubRuleSetter::InitializeZmqSocket() {
    socket_.reset(new zmq::socket_t(*context_, ZMQ_REQ));
    std::string full_address = "tcp://" + ipPort_;
    socket_->connect(full_address.data());

    //  Configure socket to not wait at close time
    int linger = 0;
    socket_->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
}

bool
OssStubRuleSetter::TrySendRequestAndReceiveReply(std::shared_ptr<rpc::OSSCommandMessage> &msg, std::string &reply) {
    std::string raw_msg = msg->SerializeAsString();
    if (!SendAndReceiveRequest(raw_msg, reply)) {
        spdlog::get("console")->error("couldn't communicate with OSS stub at {}", ipPort_);
        return false;
    }
    return true;
}

bool OssStubRuleSetter::SendAndReceiveRequest(std::string &raw_msg, std::string &reply) {
    int retries_left = kRequestRetries;

    while (retries_left) {
        s_send(*socket_, raw_msg);

        while (1) {
            //  Poll socket for a reply, with timeout
            zmq::pollitem_t items[] = {{(void *) *socket_, 0, ZMQ_POLLIN, 0}};
            zmq::poll(&items[0], 1, kRequestTimeoutUs);

            //  If we got a reply, process it
            if (items[0].revents & ZMQ_POLLIN) {
                reply = s_recv(*socket_);
                return true;
            } else if (--retries_left == 0) {
                spdlog::get("console")->error("server seems to be offline, abandoning");
                return false;
            } else {
                spdlog::get("console")->warn("no response from server, retrying...");
                //  Old socket will be confused; close it and open a new one
                InitializeZmqSocket();
                s_send(*socket_, raw_msg.data());
            }
        }
    }

    return true;
}

std::shared_ptr<rpc::OSSCommandMessage> OssStubRuleSetter::BuildCommandMessage(const std::string &cmd) const {
    auto msg = std::make_shared<rpc::OSSCommandMessage>();
    msg->set_type(rpc::OSSCommandMessage::REQUEST);
    msg->mutable_request()->set_command(cmd);
    return msg;
}

bool OssStubRuleSetter::ProcessReply(std::string &reply) {

    rpc::OSSCommandMessage msg;
    if (!msg.ParseFromString(reply))
        spdlog::get("console")->error("got invalid answer");
    else
        spdlog::get("console")->debug("got answer: {}", msg.DebugString());

    if (msg.reply().rc() != 0) {
        spdlog::get("console")->error("setting rule on {} failed: {}", ipPort_, msg.reply().return_msg());
        return false;
    }

    return true;
}


}
