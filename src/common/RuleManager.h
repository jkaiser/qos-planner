//
// Created by Jürgen Kaiser on 02.11.16.
//

#ifndef QOS_PLANNER_RULEMANAGER_H
#define QOS_PLANNER_RULEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>

#include "RuleSetter.h"
#include "Lustre.h"

namespace common {
class RuleManager {

private:

    struct RuleDefinition {
        std::string rule_name;
        std::vector<std::string> ost_ips;

        RuleDefinition(std::string &rname, std::vector<std::string> &ips) : rule_name(rname), ost_ips(ips){};
    };

    std::shared_ptr<RuleSetter> rule_setter_;
    std::shared_ptr<Lustre> lustre_;

    const std::string lustre_tbf_rule_postfix_ = "_qosp";

    std::unordered_map<std::string, std::shared_ptr<RuleDefinition>> active_rules_;

    bool SetRulesAsync(uint32_t min_throughput_mbs, std::vector<std::string> &ost_ips, std::string &job_id,
                           std::string &rule_name) const;

    void RemoveRulesAsync(const std::string &job_id, const std::string &rule_name, const std::vector<std::string> &ost_ips) const;

public:

    RuleManager(std::shared_ptr<RuleSetter> rule_setter, std::shared_ptr<Lustre> lustre);

    bool SetRules(std::vector<std::string> &ost_ips, std::string &job_id, uint32_t min_throughput_mbs);

    bool RemoveRules(std::string &job_id);
};
}

#endif //QOS_PLANNER_RULEMANAGER_H
