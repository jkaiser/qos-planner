//
// Created by Jürgen Kaiser on 03.10.16.
//

#include "ReserveRequestBuilder.h"
#include <sstream>

#include <spdlog/spdlog.h>

bool
ReserveRequestBuilder::Parse(const std::string &filenames, int throughput, const std::string &time_end,
                             rpc::Request &request) const {

    if (filenames.empty() || (throughput == 0) || time_end.empty()) {
        return false;
    }

    int tend = 0;
    if (!tryParseIntVals(time_end, tend)) {
        return false;
    }

    request.set_type(rpc::Request::Type::Request_Type_RESERVE);
    request.mutable_resourcerequest()->set_throughputmb(throughput);
    request.mutable_resourcerequest()->set_tstop(tend);

    addFilenames(filenames, request);
    return true;
}

bool
ReserveRequestBuilder::tryParseIntVals(const std::string &time_end,
                                       int &tend) const {
    try {
        tend = stoi(time_end);
    } catch (...) {
        spdlog::get("console")->debug("invalid values given");
        return false;
    }
    return true;
}

void ReserveRequestBuilder::addFilenames(const std::string &filenames, rpc::Request &request) const {
    if (filenames.find(',') == -1) {
        request.mutable_resourcerequest()->add_files(filenames);
    } else {
        std::__1::stringstream ss(filenames);
        std::__1::string fname;
        while (getline(ss, fname, ',')) {
            request.mutable_resourcerequest()->add_files(fname);
        }
    }
}
