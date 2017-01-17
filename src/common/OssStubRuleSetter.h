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

#ifndef QOS_PLANNER_OSSTUBRULESETTER_H
#define QOS_PLANNER_OSSTUBRULESETTER_H

#include "RuleSetter.h"

#include <zhelpers.hpp>
#include "rpc/proto/oss_server_message.pb.h"

namespace common {

class OssStubRuleSetter : public RuleSetter {

private:
    const std::string kDefaultPort = "5556";

    std::string ipPort_;
    std::shared_ptr<zmq::socket_t> socket_;
    zmq::context_t *context_;
    const int kRequestTimeoutUs = 1000;
    const int kRequestRetries = 3;


    void InitializeZmqSocket();

    std::shared_ptr<rpc::OSSCommandMessage> BuildCommandMessage(const std::string &cmd) const;

    bool TrySendRequestAndReceiveReply(std::shared_ptr<rpc::OSSCommandMessage> &msg, std::string &reply);

    bool SendAndReceiveRequest(std::string &raw_msg, std::string &reply);

    bool ProcessReply(std::string &reply);


    bool RemotelyExecuteCmd(const std::string &ip, const std::string &cmd);

public:

    OssStubRuleSetter();

    ~OssStubRuleSetter();

    virtual bool SetRule(const std::string &ip, const std::string &job_id, const std::string &rule_name,
                         uint32_t rpc_rate_limit) override;

    virtual bool RemoveRule(const std::string &ip, const std::string &rule_name, const std::string &job_id) override;

};
}

#endif //QOS_PLANNER_OSSTUBRULESETTER_H
