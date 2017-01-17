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

#ifndef QOS_PLANNER_STORAGEREQPARSER_H
#define QOS_PLANNER_STORAGEREQPARSER_H


#include <istream>
#include <map>

#include <nlohmann/json.hpp>

namespace common {

class StorageReqParser {

private:

    std::string read_files_;
    int throughput_mbs_;

    bool tryToParse(std::istream &in_stream, nlohmann::json &j) const;
    void ParseContent(const nlohmann::json &j);
public:
    const std::string &getRead_files() const;
    int getThroughput_mbs() const;

    bool Parse(std::istream &in_stream);

};

}

#endif //QOS_PLANNER_STORAGEREQPARSER_H
