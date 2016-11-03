//
// Created by Jürgen Kaiser on 02.11.16.
//

#ifndef QOS_PLANNER_SSHRULESETTER_H
#define QOS_PLANNER_SSHRULESETTER_H

#include "RuleSetter.h"

namespace common {

class SSHRuleSetter : public RuleSetter {
public:
    virtual bool SetRule(const std::string &ip, const std::string &job_id, const std::string &rule_name,
                         uint32_t rpc_rate_limit) override;

    virtual bool RemoveRule(const std::string &ip, const std::string &rule_name, const std::string &job_id) override;
};

}

#endif //QOS_PLANNER_SSHRULESETTER_H
