//
// Created by Jürgen Kaiser on 02.11.16.
//

#include "RuleManager.h"
#include <utility>
#include <future>
#include <thread>
#include <unordered_set>

namespace common {

RuleManager::RuleManager(std::shared_ptr<RuleSetter> rule_setter, std::shared_ptr<OstIpsCache> ost_ip_cache,
                         std::shared_ptr<OstIdsConverter> ost_id_converter) : rule_setter_(rule_setter),
                                                                              ost_ip_cache_(ost_ip_cache),
                                                                              ost_id_converter_(ost_id_converter) {}

bool RuleManager::SetRules(std::vector<std::string> &ost_ids, const std::string &job_id,
                           uint32_t min_throughput_mbs) {
    if (ost_ids.empty() || (active_rules_.find(job_id) != active_rules_.end())) {
        return false;
    }

    std::vector<std::string> ost_ips;
    ost_ips.reserve(ost_ids.size());
    if (!determineOstIps(ost_ids, ost_ips)) {
        return false;
    }

    /**
     * Experiments showed that NRS rules are global for an OSS, i.e. a rule for 50MB/s for an OSS gives 50MB/s
     * independently from the number of OSTs behind the OSS. However, min_throughput_mbs is interpreted as _per OST_,
     * therefore we must adapt this value accordingly.
     */
    std::unordered_map<std::string, uint32_t> oss_to_mbs_map;
    std::vector<std::string> unique_ost_ips;
    computeOssToMBsAndCountUnique(min_throughput_mbs, ost_ips, oss_to_mbs_map, unique_ost_ips);

    std::string rule_name = lustre_tbf_rule_postfix_ + job_id;

    if (!SetRulesAsync(oss_to_mbs_map, job_id, rule_name)) {
        return false;
    }

    active_rules_[job_id] = std::make_shared<RuleDefinition>(rule_name, unique_ost_ips);
    return true;
}

void RuleManager::computeOssToMBsAndCountUnique(uint32_t min_throughput_mbs, const std::vector<std::string> &ost_ips,
                                                std::unordered_map<std::string, uint32_t> &oss_to_mbs_map,
                                                std::vector<std::string> &unique_ost_ips) const {
    for (auto &ip : ost_ips) {
        auto it = oss_to_mbs_map.find(ip);
        if (it != oss_to_mbs_map.end()) {
            it->second += min_throughput_mbs;
        } else {
            oss_to_mbs_map[ip] = min_throughput_mbs;
            unique_ost_ips.push_back(ip);
        }
    }
}

bool RuleManager::determineOstIps(const std::vector<std::string> &ost_ids,
                                  std::vector<std::string> &ost_ips) const {
    for (auto &id : ost_ids) {
        std::string ip = ost_ip_cache_->GetIp(ost_id_converter_->ToUUID(id));
        if (ip.empty()) {
            return false;
        }
        ost_ips.push_back(ip);
    }
    return true;
}

bool RuleManager::SetRulesAsync(const std::unordered_map<std::string, uint32_t> &ost_ip_to_count_map,
                                const std::string &job_id,
                                std::string &rule_name) const {
    std::vector<std::future<bool>> result_futures(ost_ip_to_count_map.size());

    int index = 0;
    for (auto &entry: ost_ip_to_count_map) {
        result_futures[index] = async(&RuleSetter::SetRule, rule_setter_, entry.first, job_id, rule_name,
                                  entry.second);
        ++index;
    }

    std::vector<std::future<bool>> cleanup_tasks;
    index = 0;
    for (auto &entry: ost_ip_to_count_map) {
        if (!result_futures[index].get()) {
            cleanup_tasks.push_back(async(&RuleSetter::RemoveRule, rule_setter_, entry.first, rule_name, job_id));
        }
        ++index;
    }

    for (auto &task : cleanup_tasks) {
        task.wait();
    }
    return cleanup_tasks.empty();
}

bool RuleManager::RemoveRules(const std::string &job_id) {

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
