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

#include "StorageReqParser.h"

#include <spdlog/spdlog.h>

namespace common {

bool StorageReqParser::Parse(std::istream &in_stream) {
    nlohmann::json j;

    if (!tryToParse(in_stream, j))
        return false;

    if (j.empty()) {
        return true;
    }

    ParseContent(j);

    return true;
}

bool StorageReqParser::tryToParse(std::istream &in_stream, nlohmann::json &j) const {
    try {
        j << in_stream;
    } catch (const std::exception & ex) {
//        spdlog::get("console")->critical("couldn't parse storage req. file. error: {}", ex.what());
        return false;
    }
    return true;
}

const std::string &StorageReqParser::getRead_files() const {
    return read_files_;
}

int StorageReqParser::getThroughput_mbs() const {
    return throughput_mbs_;
}

void StorageReqParser::ParseContent(const nlohmann::json &j) {

    auto throughput = j.find("throughput_mbs");
    if (throughput != j.end()) {
        throughput_mbs_ = throughput->get<int>();

    }

    auto read_files = j.find("read_files");
    if (read_files != j.end()) {
        std::stringstream ss;
        const char* separator = "";
        for (auto &rfile : *read_files) {
            ss << separator << rfile.get<std::string>();
            separator = ",";
        }
        read_files_ = ss.str();
    }
}

}
