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

#include "OSTLimitConfigParser.h"

#include <spdlog/spdlog.h>

namespace common {

bool OSTLimitConfigParser::Parse(std::istream &in_stream) {

    limits_.clear();

    nlohmann::json j;

    if (!tryToParse(in_stream, j))
        return false;

    if (j.empty()) {
        return true;
    }

    ParseContent(j);

    return true;
}

void OSTLimitConfigParser::ParseContent(const nlohmann::json &j) {
    for (auto &element : j) {
        auto name_it = element.find("name");
        auto max_mb_per_sec_it = element.find("max_mbs");
        if ((name_it != element.end()) && (max_mb_per_sec_it != element.end())) {
            limits_[*name_it] = max_mb_per_sec_it->get<double>();
        }
    }
}

bool OSTLimitConfigParser::tryToParse(std::istream &in_stream, nlohmann::json &j) const {
    try {
        j << in_stream;
    } catch (const std::exception & ex) {
        spdlog::get("console")->critical("couldn't parse ost limits file. error: {}", ex.what());
        return false;
    }
    return true;
}

std::map<std::string, float> OSTLimitConfigParser::GetLimits() {
    return limits_;
}

}