//
// Created by Jürgen Kaiser on 02.11.16.
//

#ifndef QOS_PLANNER_RULEMANAGER_H
#define QOS_PLANNER_RULEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "RuleSetter.h"
#include "Lustre.h"
#include "OstIpsCache.h"
#include "OstIdsConverter.h"

namespace common {
class RuleManager {

private:

    struct RuleDefinition {
        std::string rule_name;
        std::vector<std::string> ost_ips;

        RuleDefinition(std::string &rname, std::vector<std::string> &ips) : rule_name(rname), ost_ips(ips){};
    };

    std::shared_ptr<RuleSetter> rule_setter_;

    std::shared_ptr<OstIpsCache> ost_ip_cache_;
    std::shared_ptr<OstIdsConverter> ost_id_converter_;

    const std::string lustre_tbf_rule_postfix_ = "_qosp";

    std::unordered_map<std::string, std::shared_ptr<RuleDefinition>> active_rules_;

    bool SetRulesAsync(const std::unordered_map<std::string, uint32_t> &ost_ip_to_count_map,
                       const std::string &job_id,
                       std::string &rule_name) const;

    void RemoveRulesAsync(const std::string &job_id, const std::string &rule_name, const std::vector<std::string> &ost_ips) const;

    bool determineOstIps(const std::vector<std::string> &ost_ids, std::vector<std::string> &ost_ips) const;

    void computeOssToMBsAndCountUnique(uint32_t min_throughput_mbs, const std::vector<std::string> &ost_ips,
                                       std::unordered_map<std::string, uint32_t> &oss_to_mbs_map,
                                       std::vector<std::string> &unique_ost_ips) const;
protected:

    RuleManager() {};

public:

    RuleManager(std::shared_ptr<RuleSetter> rule_setter, std::shared_ptr<OstIpsCache> ost_ip_cache,
                std::shared_ptr<OstIdsConverter> ost_id_converter);

    virtual bool SetRules(std::vector<std::string> &ost_ids, const std::string &job_id, uint32_t min_throughput_mbs);

    virtual bool RemoveRules(const std::string &job_id);
};
}

#endif //QOS_PLANNER_RULEMANAGER_H
