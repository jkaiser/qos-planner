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

#ifndef QOS_PLANNER_OSSRULESERVER_H
#define QOS_PLANNER_OSSRULESERVER_H

#include <string>

#include <zhelpers.hpp>
#include <rpc/proto/oss_server_message.pb.h>

namespace oss_rule_server {

/**
 * The OSSRuleServer is a small instance that runs on a node running a Lustre OSS instance. The purpose is to execute
 * remote commands cheaper than via ssh.
 */
class OSSRuleServer {

private:
    std::string ip_port_;
    std::shared_ptr<zmq::socket_t> server_;

    zmq::context_t *context_;

    void initZMQ();

    void ProcessUnparseableMsg(rpc::OSSCommandMessage &msg) const;

    bool ProcessMessage(rpc::OSSCommandMessage &msg) const;

    bool ServeReply(rpc::OSSCommandMessage &message) const;

    bool ServeCommand(rpc::OSSCommandMessage &message) const;

public:
    OSSRuleServer(const std::string &ip_port);

    bool Init();

    bool TearDown();

    void Serve();

};

}

#endif //QOS_PLANNER_OSSRULESERVER_H
