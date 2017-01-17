/*
 * Copyright (c) 2017 Jürgen Kaiser
 */
/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * GPL HEADER END
 */

#include "ReserveRequestBuilder.h"
#include <sstream>

#include <spdlog/spdlog.h>

bool
ReserveRequestBuilder::BuildRequest(const std::string &id, const std::string &filenames, int throughput,
                                    int duration_sec,
                                    rpc::Request &request) const {

    if (duration_sec <= 0) {
        return false;
    }

    request.set_type(rpc::Request::Type::Request_Type_RESERVE);
    request.mutable_resourcerequest()->set_id(id);
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
