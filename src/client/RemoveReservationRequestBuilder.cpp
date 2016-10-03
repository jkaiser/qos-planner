//
// Created by Jürgen Kaiser on 03.10.16.
//

#include "RemoveReservationRequestBuilder.h"

bool RemoveReservationRequestBuilder::BuildRequest(const std::string &id, rpc::Request &request) const {
    request.set_type(rpc::Request_Type_DELETE);
    request.mutable_deleterequest()->add_id(id);

    return true;
}
