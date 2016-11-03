//
// Created by Jürgen Kaiser on 02.11.16.
//

#include "RuleManager.h"
#include <utility>
#include <future>
#include <thread>

namespace common {

RuleManager::RuleManager(std::shared_ptr<RuleSetter> rule_setter, std::shared_ptr<Lustre> lustre) :
        rule_setter_(rule_setter),
        lustre_(lustre) {}

bool RuleManager::SetRules(std::vector<std::string> &ost_ips, std::string &job_id,
                           uint32_t min_throughput_mbs) {
    if (ost_ips.empty() || (active_rules_.find(job_id) != active_rules_.end())) {
        return false;
    }

    std::string rule_name = lustre_tbf_rule_postfix_ + job_id;

    if (SetRulesAsync(min_throughput_mbs, ost_ips, job_id, rule_name)) {
        active_rules_[job_id] = std::make_shared<RuleDefinition>(rule_name, ost_ips);
        return true;
    }
    return false;
}

bool RuleManager::SetRulesAsync(uint32_t min_throughput_mbs, std::vector<std::string> &ost_ips, std::string &job_id,
                                std::string &rule_name) const {
    std::vector<std::future<bool>> result_futures(ost_ips.size());
    for (int i = 0; i < ost_ips.size(); ++i) {
        result_futures[i] = async(&RuleSetter::SetRule, rule_setter_, ost_ips[i], job_id, rule_name,
                                  min_throughput_mbs);
    }

    std::vector<std::future<bool>> cleanup_tasks;
    for (int i = 0; i < ost_ips.size(); ++i) {
        if (!result_futures[i].get()) {
            cleanup_tasks.push_back(async(&RuleSetter::RemoveRule, rule_setter_, ost_ips[i], rule_name, job_id));
        }
    }

    for (auto &task : cleanup_tasks) {
        task.wait();
    }
    return cleanup_tasks.empty();
}

bool RuleManager::RemoveRules(std::string &job_id) {

    auto it = active_rules_.find(job_id);
    if (it == active_rules_.end()) {
        return false;
    }

    RemoveRulesAsync(job_id, it->second->rule_name, it->second->ost_ips);

    active_rules_.erase(it);
    return true;
}

void RuleManager::RemoveRulesAsync(const std::string &job_id, const std::string &rule_name,
                                   const std::vector<std::string> &ost_ips) const {
    std::vector<std::future<bool>> result_futures(ost_ips.size());
    for (int i = 0; i < ost_ips.size(); ++i) {
        result_futures[i] = async(&RuleSetter::RemoveRule, rule_setter_, ost_ips[i], rule_name, job_id);
    }

    for (auto &task : result_futures) {
        task.wait();
    }
}
}
