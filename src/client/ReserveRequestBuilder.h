//
// Created by Jürgen Kaiser on 03.10.16.
//

#ifndef QOS_PLANNER_REQUESTBUILDER_H
#define QOS_PLANNER_REQUESTBUILDER_H

#include <string>
#include <rpc/proto/message.pb.h>

class ReserveRequestBuilder {

public:
    bool Parse(const std::string &filenames, int throughput, int duration, rpc::Request &request) const;

    void addFilenames(const std::string &filenames, rpc::Request &request) const;
};


#endif //QOS_PLANNER_REQUESTBUILDER_H
