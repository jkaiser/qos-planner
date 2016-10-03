//
// Created by Jürgen Kaiser on 03.10.16.
//

#ifndef QOS_PLANNER_LISTRESERVATIONSREQUESTBUILDER_H
#define QOS_PLANNER_LISTRESERVATIONSREQUESTBUILDER_H

#include <rpc/proto/message.pb.h>

class ListReservationsRequestBuilder {

public:
    bool BuildRequest(rpc::Request &request) const;
};


#endif //QOS_PLANNER_LISTRESERVATIONSREQUESTBUILDER_H
