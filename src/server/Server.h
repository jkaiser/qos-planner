//
// Created by jkaiser on 29.03.16.
//

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
    std::string root_path;
    std::string ip_port_;
    std::shared_ptr<common::Planner> planner;
    std::shared_ptr<zmq::socket_t> server;

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
};

#endif //QOS_PLANNER_SERVER_H
