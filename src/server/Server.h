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

#ifndef QOS_PLANNER_SERVER_H
#define QOS_PLANNER_SERVER_H

#include <memory>
#include <string>

#include <Planner.h>

#include <zhelpers.hpp>
#include "../common/rpc/proto/message.pb.h"

/**
 * Server s(...);
 * s.Init();
 * s.Serve(); // blocking call
 *
 * // in an interrupt handler;
 * <<Interrupt (SigTerm)>>
 * s.Teardown();    // stops all future requests.
 */
class Server {

private:
    std::string root_path_;
    std::string ip_port_;
    std::shared_ptr<common::Planner> planner_;
    std::shared_ptr<zmq::socket_t> server_;

    zmq::context_t *context_;
    void initZMQ();

public:
    /**
     * Constructor
     *
     * root_path:   The path where the server will store permanent data structures. If empty, it will only
     *              uses in-memory data structures.
     */
    Server(const std::string &ip_port, const std::string &root_path, std::shared_ptr<common::Planner> planner);
    bool Init();

    bool TearDown();

    /**
     * ip_port: The ip-port pair to listen to. Example: "192.168.1.1:1234"
     */
    void Serve();

    void ProcessUnparsableMsg(rpc::Message &msg) const;

    bool ProcessMessage(rpc::Message &msg) const;
};

#endif //QOS_PLANNER_SERVER_H
