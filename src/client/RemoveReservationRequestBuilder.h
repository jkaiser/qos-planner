//
// Created by Jürgen Kaiser on 03.10.16.
//

#ifndef QOS_PLANNER_REMOVERESERVATIONREQUESTBUILDER_H
#define QOS_PLANNER_REMOVERESERVATIONREQUESTBUILDER_H

#include <string>
#include <rpc/proto/message.pb.h>

class RemoveReservationRequestBuilder {

public:
    bool BuildRequest(const std::string &id, rpc::Request &request) const;
};


#endif //QOS_PLANNER_REMOVERESERVATIONREQUESTBUILDER_H
