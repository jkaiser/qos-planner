//
// Created by jkaiser on 08.03.16.
//


#ifndef QOS_PLANNER_CLIENT_H
#define QOS_PLANNER_CLIENT_H


#include <iostream>
#include <sstream>
#include <string>

#include <zhelpers.hpp>

#include "../common/rpc/proto/message.pb.h"


static zmq::socket_t * s_client_socket(const std::string ipPort, zmq::context_t & context);
static bool sendAndReceive(const std::string ipPort, const std::string &rawMsg, std::string &reply);

class Client {

private:
    std::string ipPort;
    std::shared_ptr<zmq::socket_t> client;
    zmq::context_t *context;
    const int request_timeout = 1000;
    const int request_retries = 3;

    rpc::Message buildMessage() const;

    bool sendAndReceiveRequest(std::string &raw_msg, std::string &reply);

    void InitializeZMQSocket();

    void ProcessReply(std::string &reply);

    bool trySendRequestAndReceiveReply(const std::shared_ptr<rpc::Request> &request, std::string &reply);

    bool IsInputValid(const std::string &filenames, const std::string &tStart) const;

public:

    Client(std::string ipPort);

    bool Init();
    bool requestResources(const std::string &filenames, int throughput, const std::string &Tstart);
    bool requestResources(std::string request);
};



#endif //QOS_PLANNER_CLIENT_H
