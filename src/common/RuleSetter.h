//
// Created by Jürgen Kaiser on 02.11.16.
//

#ifndef QOS_PLANNER_RULESETTER_H
#define QOS_PLANNER_RULESETTER_H

#include <string>
#include <vector>

namespace common {


class RuleSetter {

public:
    virtual bool SetRule(const std::string &ip, const std::string &job_id, const std::string &rule_name,
                         uint32_t rpc_rate_limit) = 0;
    virtual bool RemoveRule(const std::string &ost_ip, const std::string &rule_name, const std::string &job_id) = 0;

};

}

#endif //QOS_PLANNER_RULESETTER_H
