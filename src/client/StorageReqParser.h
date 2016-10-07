//
// Created by Jürgen Kaiser on 05.10.16.
//

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
    int getDuration() const;
    int getThroughput_mbs() const;

    bool Parse(std::istream &in_stream);

};

}

#endif //QOS_PLANNER_STORAGEREQPARSER_H
