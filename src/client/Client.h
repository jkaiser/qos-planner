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


#ifndef QOS_PLANNER_CLIENT_H
#define QOS_PLANNER_CLIENT_H


#include <iostream>
#include <sstream>
#include <string>

#include <zhelpers.hpp>

#include "../common/rpc/proto/message.pb.h"


class Client {

private:
    std::string ipPort_;
    std::shared_ptr<zmq::socket_t> client_;
    zmq::context_t *context_;
    const int kRequestTimeoutUs = 1000;
    const int kRequestRetries = 3;

    bool SendAndReceiveRequest(std::string &raw_msg, std::string &reply);

    void InitializeZMQSocket();

    void ProcessReply(std::string &reply);

    bool TrySendRequestAndReceiveReply(std::shared_ptr<rpc::Message> &request, std::string &reply);

    bool IsInputValid(const std::string &id, const std::string &filenames, int duration) const;

    bool TryBuildReserveMessage(const std::string &id, const std::string &filenames, int throughput, int duration,
                                std::shared_ptr<rpc::Message> &msg) const;

    bool ProcessListReply(const std::string &reply) const;

    bool TryParseReqFile(const std::string &filename, std::string &filenames_to_use, int &throughput_to_use);

public:

    Client(std::string ipPort);
    bool Init();
    bool RemoveReservation(const std::string &reservation_id);
    bool ListReservations();
    bool TryReserveResources(const std::string &id, const std::string &filenames, int throughput, int duration_sec, const std::string &storage_req_file);
};



#endif //QOS_PLANNER_CLIENT_H
