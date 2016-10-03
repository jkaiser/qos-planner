//
// Created by Jürgen Kaiser on 03.10.16.
//

#include "ReserveRequestBuilder.h"
#include <sstream>

#include <spdlog/spdlog.h>

bool
ReserveRequestBuilder::Parse(const std::string &filenames, int throughput, int duration_sec,
                             rpc::Request &request) const {

    if (duration_sec <= 0) {
        return false;
    }

    request.set_type(rpc::Request::Type::Request_Type_RESERVE);
    request.mutable_resourcerequest()->set_throughputmb(throughput);
    request.mutable_resourcerequest()->set_durationsec(duration_sec);

    addFilenames(filenames, request);
    return true;
}

void ReserveRequestBuilder::addFilenames(const std::string &filenames, rpc::Request &request) const {
    if (filenames.empty()) {
        request.mutable_resourcerequest();
    } else if (filenames.find(',') == -1) {
        request.mutable_resourcerequest()->add_files(filenames);
    } else {
        std::stringstream ss(filenames);
        std::string fname;
        while (getline(ss, fname, ',')) {
            request.mutable_resourcerequest()->add_files(fname);
        }
    }
}
