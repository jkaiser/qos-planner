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


class Client {

private:
    std::string ipPort;
    std::shared_ptr<zmq::socket_t> client;
    zmq::context_t *context;
    const int request_timeout = 1000;
    const int request_retries = 3;

    bool SendAndReceiveRequest(std::string &raw_msg, std::string &reply);

    void InitializeZMQSocket();

    void ProcessReply(std::string &reply);

    bool TrySendRequestAndReceiveReply(std::shared_ptr<rpc::Message> &request, std::string &reply);

    bool IsInputValid(const std::string &id, const std::string &filenames, int duration) const;

    bool TryBuildReserveMessage(const std::string &id, const std::string &filenames, int throughput, int duration,
                                std::shared_ptr<rpc::Message> &msg) const;

    bool ProcessListReply(const std::string &reply) const;

    bool TryParseReqFile(const std::string &filename, std::string &filenames_to_use, int &throughput_to_use, int &duration_to_use);

public:

    Client(std::string ipPort);
    bool Init();
    bool RemoveReservation(const std::string &reservation_id);
    bool ListReservations();
    bool TryReserveResources(const std::string &id, const std::string &filenames, int throughput, int duration_sec, const std::string &storage_req_file);
};



#endif //QOS_PLANNER_CLIENT_H
