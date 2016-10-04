//
// Created by Jürgen Kaiser on 01.10.16.
//

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