//
// Created by Jürgen Kaiser on 01.10.16.
//

#ifndef QOS_PLANNER_OSTLIMITCONFIGPARSER_H
#define QOS_PLANNER_OSTLIMITCONFIGPARSER_H

#include <istream>
#include <map>

#include <nlohmann/json.hpp>

namespace common {
class OSTLimitConfigParser {

private:
    std::map<std::string, float> limits_;

    bool tryToParse(std::istream &in_stream, nlohmann::json &j) const;
    void ParseContent(const nlohmann::json &j);
public:

    bool Parse(std::istream &in_stream);

    std::map<std::string, float> GetLimits();
};
}


#endif //QOS_PLANNER_OSTLIMITCONFIGPARSER_H
