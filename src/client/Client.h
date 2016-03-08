//
// Created by jkaiser on 08.03.16.
//


#ifndef QOS_PLANNER_CLIENT_H
#define QOS_PLANNER_CLIENT_H


#include <iostream>
#include <sstream>
#include <string>

#include <zhelpers.hpp>

#include "../../../../../Library/Caches/CLion12/cmake/generated/a7b7e32b/a7b7e32b/Debug/src/common/rpc/proto/message.pb.h"


static zmq::socket_t * s_client_socket(const std::string ipPort, zmq::context_t & context);
static bool sendAndReceive(const std::string ipPort, const std::string &rawMsg, std::string reply);

class Client {

private:
    std::string ipPort;

public:
    Client(std::string ipPort);

    bool Init() {
        return true;
    }

    bool requestResources(std::string request);

};



#endif //QOS_PLANNER_CLIENT_H
