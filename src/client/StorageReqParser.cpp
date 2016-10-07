//
// Created by Jürgen Kaiser on 05.10.16.
//

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
